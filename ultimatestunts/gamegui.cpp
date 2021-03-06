/***************************************************************************
                          gamegui.cpp  -  The Game menu interface
                             -------------------
    begin                : do okt 14 2004
    copyright            : (C) 2004 by CJP
    email                : cornware-cjp@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

//I18n
#include <libintl.h>
#define _(String) gettext (String)
#define N_(String1, String2, n) ngettext ((String1), (String2), (n))

#include "SDL.h"

//Generic stuff
#include "lconfig.h"
#include "usmacros.h"
#include "usmisc.h"
#include "datafile.h"

#include "metaserver.h"

//Track editor stuff
#include "trackdocument.h"
#include "tegui.h"

//GUI elements
#include "gamegui.h"
#include "menu.h"
#include "longmenu.h"
#include "label.h"
#include "renderwidget.h"

//Simulation data
#include "objectchoice.h"
#include "aiplayercar.h"
#include "humanplayer.h"
#include "hiscorefile.h"

CUSCore *_USGameCore = NULL;
CGUIPage *_LoadingPage = NULL;

bool game_mainloop()
{
	return _USGameCore->update();
}

bool loadingCallback(const CString &status, float progress)
{
	CMenu *menu = (CMenu *)(_LoadingPage->m_Widgets[0]);

	if(status != "")
		menu->m_Lines[0] = _("Status: ") + status;

	//TODO: progress bar

	_LoadingPage->onRedraw();
	theWinSystem->swapBuffers();

	//Check for escape key
	bool exit = false;
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
			if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
			{
				exit = true;
			}
	}

	if(exit) printf("EXIT!!!!!!!\n");

	return !exit;
}

/*
Small convenience class to add some functionality
to the track editor
*/
class CTEGUI_InGameGUI : public CTEGUI
{
public:
	CTEGUI_InGameGUI(CWinSystem *winsys, CSound *soundsys) : CTEGUI(winsys)
	{
		m_SoundSystem = soundsys;
	}

	virtual int onKeyPress(int key)
	{
		CGameWinSystem *winsys = (CGameWinSystem *)m_WinSys;
		if((unsigned int)key == winsys->getKeyFromGlobalKey(eNextSong))
			m_SoundSystem->playNextSong();

		return CTEGUI::onKeyPress(key);
	}

	virtual int onIdle()
	{
		m_SoundSystem->update();
		return CTEGUI::onIdle();
	}

protected:
	CSound *m_SoundSystem;
};

CGameGUI::CGameGUI(CGameWinSystem *winsys) : CGUI(winsys)
{
	printf("---Sound system\n");
	m_SoundSystem = new CSound;


	m_GameCore = new CUSCore(winsys, m_SoundSystem);

	m_Server = NULL;

	m_ChildWidget = &m_MainPage;

	//Car file cache:
	vector<CString> theCarFiles = getDataDirContents("cars", ".conf");
	for(unsigned int i=0; i < theCarFiles.size(); i++)
	{
		SCarFile cf;
		cf.filename = "cars/" + theCarFiles[i];
		CDataFile dfile(cf.filename);
		CLConfig carconf(dfile.useExtern());
		cf.fullname = carconf.getValue("description", "fullname");
		cf.defaultColor = carconf.getValue("skin", "defaultcolor").toVector();
		//printf("file %s name %s\n", cf.filename.c_str(), cf.fullname.c_str());
		m_CarFiles.push_back(cf);
	}

	//default values:
	m_GameType = LocalGame;
	m_TrackFile = "tracks/default.track";
	m_ReplayFile = "tracks/default.repl";
	m_HostName = "localhost";
	m_HostPort = DEFAULTPORT;
	m_MinPlayers = 2;
	m_ServerName = "Ultimate Stunts"; //note the space in the name! (problem in windows command lines?)

	//Default players:
	SPlayerDescr pd;
	pd.name = _("Anonymous");
	pd.isHuman = true;
	pd.carColor = CVector(1, 0, 0); //default: red
	pd.automaticGearbox = true;
	for(unsigned int i=0; i < m_CarFiles.size(); i++)
		if(m_CarFiles[i].filename == "cars/ferrarispider.conf")
		{
			pd.carIndex = i;
			break;
		}
	m_PlayerDescr.push_back(pd);
	m_SelectedPlayer = 0;

	m_OptionsAreChanged = false;

	//Load an initial state
	m_GameCore->initLocalGame(m_TrackFile);

	//setting up the menus:
	//MAIN MENU
	m_MainPage.m_DrawBackground = true;
	CMenu *menu = new CMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_MainPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = true;

	//GAMETYPE MENU
	m_GameTypePage.m_DrawBackground = true;
	menu = new CMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_GameTypePage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = true;

	//SELECT SERVER MENU
	m_SelectServerPage.m_DrawBackground = true;
	menu = new CMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_SelectServerPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = false;

	//TRACK MENU
	m_TrackPage.m_DrawBackground = true;
	CLabel *label = new CLabel;
	label->m_Xrel = 0.1;
	label->m_Yrel = 0.75;
	label->m_Wrel = 0.8;
	label->m_Hrel = 0.05;
	m_TrackPage.m_Widgets.push_back(label);
	menu = new CMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.05;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.15;
	m_TrackPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = true;

	//LOAD TRACK MENU
	m_LoadTrackPage.m_DrawBackground = true;
	menu = new CLongMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_LoadTrackPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = false;

	//REPLAY MENU
	m_ReplayPage.m_DrawBackground = true;
	menu = new CMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_ReplayPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = false;

	//PLAYERS MENU
	m_PlayersPage.m_DrawBackground = true;
	menu = new CMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_PlayersPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = true;

	//PLAYER MENU
	m_PlayerPage.m_DrawBackground = true;
	{
		m_CarViewer = new CCarViewer(m_GameCore->getGraphicWorld());
		CRenderWidget *render;
		if(theMainConfig->getValue("graphics", "car_preview") == "true")
		{
			render = new CRenderWidget(true, false, true);
			render->m_Renderer = m_CarViewer;
		}
		else
		{
			render = new CRenderWidget(true, false, false);
		}

		render->m_Xrel = 0.1;
		render->m_Yrel = 0.2;
		render->m_Wrel = 0.8;
		render->m_Hrel = 0.6;
		m_PlayerPage.m_Widgets.push_back(render);
	}
	menu = new CMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_PlayerPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = true;

	//CAR MENU
	m_CarPage.m_Title = "Select a car:";
	m_CarPage.m_DrawBackground = true;
	menu = new CLongMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_CarPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = false;

	//OPTIONS MENU
	m_OptionsPage.m_DrawBackground = true;
	menu = new CLongMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_OptionsPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = true;

	//CREDITS MENU
	m_CreditsPage.m_DrawBackground = true;
	menu = new CLongMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_CreditsPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = true;

	//LOADING MENU
	m_LoadingPage.m_DrawBackground = true;
	menu = new CMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.6;
	m_LoadingPage.m_Widgets.push_back(menu);
	menu->m_Selected = 0;
	menu->m_AlignLeft = false;
	menu->m_Lines.push_back("...");

	//HISCORE MENU
	menu = new CMenu;
	menu->m_Xrel = 0.05;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.3;
	menu->m_Hrel = 0.6;
	menu->m_Selected = 21;
	menu->m_AlignLeft = true;
	m_HiscorePage.m_Widgets.push_back(menu);

	menu = new CMenu;
	menu->m_Xrel = 0.35;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.3;
	menu->m_Hrel = 0.6;
	menu->m_Selected = 21;
	menu->m_AlignLeft = true;
	m_HiscorePage.m_Widgets.push_back(menu);

	menu = new CMenu;
	menu->m_Xrel = 0.65;
	menu->m_Yrel = 0.2;
	menu->m_Wrel = 0.3;
	menu->m_Hrel = 0.6;
	menu->m_Selected = 21;
	menu->m_AlignLeft = true;
	m_HiscorePage.m_Widgets.push_back(menu);

	m_HiscorePage.m_DrawBackground = true;
	menu = new CMenu;
	menu->m_Xrel = 0.1;
	menu->m_Yrel = 0.05;
	menu->m_Wrel = 0.8;
	menu->m_Hrel = 0.15;
	menu->m_Selected = 0;
	menu->m_AlignLeft = false;
	m_HiscorePage.m_Widgets.push_back(menu);

	updateMenuTexts();
}

bool CGameGUI::reloadConfiguration()
{
	if(!m_WinSys->reloadConfiguration()) return false;
	if(!m_SoundSystem->reloadConfiguration()) return false;

	leave2DMode();
	if(!m_GameCore->reloadConfiguration()) return false;
	if(!m_CarViewer->reloadConfiguration()) return false;
	enter2DMode();

	//Set car preview
	CRenderWidget *render = (CRenderWidget *)(m_PlayerPage.m_Widgets[0]);
	if(theMainConfig->getValue("graphics", "car_preview") == "true")
	{
		render->setIdleRedraw(true);
		render->m_Renderer = m_CarViewer;
	}
	else
	{
		render->setIdleRedraw(false);
		render->m_Renderer = NULL;
	}

	//First check the validity of the datadir, before changing it
	{
		CString DataDir;
		CString cnf = theMainConfig->getValue("files", "datadir");
		if(cnf != "")
		{
			if(cnf[cnf.length()-1] != '/') cnf += '/';
			DataDir = cnf;
		}

#ifdef UNIX_TREE
		//fill in home dir for "~/"
		CString homedir = getenv("HOME");
		if(DataDir.mid(0, 2) == "~/")
			DataDir = homedir + DataDir.mid(1);
#endif

		if(inDevelopmentTree())
			DataDir = "./data/";

		CString testfile = DataDir + "misc/iso8859-1.rgba";
		if(!fileExists(testfile)) return false;
	}

	update_shared_configuration(); //sets data dir and language

	updateMenuTexts();

	return true;
}

CGameGUI::~CGameGUI()
{
	delete m_GameCore;
	delete m_SoundSystem;
	delete m_CarViewer;

	if(m_Server != NULL)
	{
		sleep(1);
		delete m_Server;
		m_Server = NULL;
	}
}

int CGameGUI::onKeyPress(int key)
{
	CGameWinSystem *winsys = (CGameWinSystem *)m_WinSys;
	if((unsigned int)key == winsys->getKeyFromGlobalKey(eNextSong))
		m_SoundSystem->playNextSong();

	return CGUI::onKeyPress(key);
}

int CGameGUI::onIdle()
{
	m_SoundSystem->update();

	return CGUI::onIdle();
}

CString CGameGUI::key2name(int key) const
{
	CGameWinSystem *winsys = (CGameWinSystem *)m_WinSys;
	return winsys->key2name(key);
}

int CGameGUI::name2key(const CString &name) const
{
	CGameWinSystem *winsys = (CGameWinSystem *)m_WinSys;
	return winsys->name2key(name);
}

void CGameGUI::updateMenuTexts()
{
	//some texts that are used more than one time:
	CString txtPlayertype[2];
	txtPlayertype[0] = _("Computer");
	txtPlayertype[1] = _("Human   ");

	//setting up the menus:
	//MAIN MENU
	m_MainPage.m_Title = _("Main menu");
	CMenu *menu = (CMenu *)(m_MainPage.m_Widgets[0]);
	menu->m_Lines.clear();
	menu->m_Lines.push_back(_("Drive!"));
	menu->m_Lines.push_back(_("Set the game type"));
	menu->m_Lines.push_back(_("Select the track"));
	menu->m_Lines.push_back(_("Players and cars"));
	menu->m_Lines.push_back(_("View a replay"));
	menu->m_Lines.push_back(_("Settings"));
	menu->m_Lines.push_back(_("Credits and License"));
	menu->m_Lines.push_back(_("Exit"));
	//add some information
	if(m_GameType == LocalGame)
	{
		menu->m_Lines[1] += CString(_(" [Local Game]"));
		menu->m_Lines[2] += CString(" [" + m_TrackFile + "]");
	}
	else if(m_GameType == NewNetwork)
	{
		menu->m_Lines[1] += CString().format(_(" [Running server %s]"), 120,
			m_ServerName.c_str(), m_HostPort);
		menu->m_Lines[2] += CString(" [") + m_TrackFile + "]";
	}
	else //JoinNetwork
	{
		menu->m_Lines[1] += CString().format(_(" [Joining game on server %s]"), 80, m_ServerName.c_str());
		menu->m_Lines[2] += CString(_(" [server will choose one]"));
	}

	int nump = int(m_PlayerDescr.size());
	menu->m_Lines[3] += CString().format(
		N_(" [%d player selected]", " [%d players selected]", nump),
		80, nump);


	//GAMETYPE MENU
	m_GameTypePage.m_Title = _("Select the game type:");
	menu = (CMenu *)(m_GameTypePage.m_Widgets[0]);
	menu->m_Lines.clear();
	menu->m_Lines.push_back(_("Local game"));
	menu->m_Lines.push_back(_("Join an existing LAN/network game"));
	menu->m_Lines.push_back(_("Start a new LAN/network game"));
	menu->m_Lines.push_back(_("Join an existing online game"));
	menu->m_Lines.push_back(_("Start a new online game"));

	//SELECT SERVER MENU
	m_SelectServerPage.m_Title = _("Select a server:");
	menu = (CMenu *)(m_SelectServerPage.m_Widgets[0]);
	menu->m_Lines.clear();
	for(unsigned int i=0; i < m_ServerList.size(); i++)
		menu->m_Lines.push_back(m_ServerList[i].hostName + " : " + m_ServerList[i].serverName);

	//TRACK MENU
	m_TrackPage.m_Title = m_TrackFile;
	menu = (CMenu *)(m_TrackPage.m_Widgets[1]);
	menu->m_Lines.clear();
	menu->m_Lines.push_back(_("Select a track"));
	menu->m_Lines.push_back(_("Edit track"));
	menu->m_Lines.push_back(_("View best time replay"));
	menu->m_Lines.push_back(_("Return to main menu"));

	//LOAD TRACK MENU
	m_LoadTrackPage.m_Title = _("Select a track");
	menu = (CMenu *)(m_LoadTrackPage.m_Widgets[0]);
	menu->m_Lines = getDataDirContents("tracks", ".track");

	//REPLAY MENU
	m_ReplayPage.m_Title = _("Select a replay file:");
	menu = (CMenu *)(m_ReplayPage.m_Widgets[0]);
	menu->m_Lines = getDataDirContents("replays", ".repl");

	//PLAYERS MENU
	m_PlayersPage.m_Title = _("Players menu:");
	menu = (CMenu *)(m_PlayersPage.m_Widgets[0]);
	menu->m_Lines.clear();
	for(unsigned int i=0; i < m_PlayerDescr.size(); i++)
	{
		CString type;
		CString entry =
			CString(i+1) + ": (" +
			txtPlayertype[m_PlayerDescr[i].isHuman] + ") " +
			m_PlayerDescr[i].name.forceLength(20) + ": " +
			m_CarFiles[m_PlayerDescr[i].carIndex].fullname;
		menu->m_Lines.push_back(entry);
	}
	menu->m_Lines.push_back(_("Add a player"));
	menu->m_Lines.push_back(_("Return to main menu"));

	//PLAYER MENU
	m_PlayerPage.m_Title = _("Configure Player:");
	{
		CString gearbox = _("manual");
		if(m_PlayerDescr[m_SelectedPlayer].automaticGearbox)
			gearbox = _("automatic");

		menu = (CMenu *)(m_PlayerPage.m_Widgets[1]);
		menu->m_Lines.clear();
		menu->m_Lines.push_back(_("Player type: ") + txtPlayertype[m_PlayerDescr[m_SelectedPlayer].isHuman]);
		menu->m_Lines.push_back(_("Name       : ") + m_PlayerDescr[m_SelectedPlayer].name);
		menu->m_Lines.push_back(_("Car        : ") + m_CarFiles[m_PlayerDescr[m_SelectedPlayer].carIndex].fullname);
		menu->m_Lines.push_back(_("Gearbox    : ") + gearbox);
		menu->m_Lines.push_back(_("Choose a car color"));
		menu->m_Lines.push_back(_("Delete this player"));
		menu->m_Lines.push_back(_("Ready"));
	}

	//CAR MENU
	m_CarPage.m_Title.format(_("Select a car for %s:"), 256, m_PlayerDescr[m_SelectedPlayer].name.c_str());
	menu = (CMenu *)(m_CarPage.m_Widgets[0]);
	menu->m_Lines.clear();
	for(unsigned int i=0; i < m_CarFiles.size(); i++)
		menu->m_Lines.push_back(m_CarFiles[i].fullname);

	//OPTIONS MENU
	m_OptionsPage.m_Title = _("Settings menu:");
	CLongMenu *lmenu = (CLongMenu *)(m_OptionsPage.m_Widgets[0]);
	lmenu->m_Lines = theMainConfig->getSections();
	lmenu->m_Lines.push_back(_("Return to main menu"));
	lmenu->m_AlignLeft = true;

	//CREDITS MENU
	m_CreditsPage.m_Title = _("Credits and License");
	lmenu = (CLongMenu *)(m_CreditsPage.m_Widgets[0]);
	lmenu->m_Lines = getCredits();

	//LOADING MENU
	m_LoadingPage.m_Title = _("Loading");

	//HISCORE MENU
	m_HiscorePage.m_Title = _("Hiscore");
	menu = (CMenu *)(m_HiscorePage.m_Widgets[3]);
	menu->m_Lines.clear();
	menu->m_Lines.push_back(_("View replay"));
	menu->m_Lines.push_back(_("Save replay"));
	menu->m_Lines.push_back(_("Play again"));
	menu->m_Lines.push_back(_("Return to main menu"));
}

void CGameGUI::start()
{
	enter2DMode();

	CString section = "mainmenu";
	while(true)
	{
		updateMenuTexts();

		if(section=="mainmenu")
			section = viewMainMenu();
		else if(section=="gametypemenu")
			section = viewGameTypeMenu();
		else if(section=="selectservermenu")
			section = viewSelectServerMenu();
		else if(section=="trackmenu")
			section = viewTrackMenu();
		else if(section=="edittrack")
			section = editTrack();
		else if(section=="loadtrackmenu")
			section = viewLoadTrackMenu();
		else if(section=="replaymenu")
			section = viewReplayMenu();
		else if(section=="viewreplay")
			section = viewReplay();
		else if(section=="playersmenu")
			section = viewPlayersMenu();
		else if(section=="playermenu")
			section = viewPlayerMenu();
		else if(section=="carmenu")
			section = viewCarMenu();
		else if(section=="optionsmenu")
			section = viewOptionsMenu();
		else if(section=="creditsmenu")
			section = viewCreditsMenu();
		else if(section=="playgame")
			section = playGame();
		else if(section=="hiscore")
			section = viewHiscore();
		else
			{printf("Error: unknown menu\n");}

		if(section == "exit" || section == "")
		{
			if(showYNMessageBox(_("Do you really want to quit?")) )
				{break;}
			else
				{section = "mainmenu";}
		}
	}

	leave2DMode();
}

CString CGameGUI::viewMainMenu()
{
	m_ChildWidget = &m_MainPage;
	if(!m_WinSys->runLoop(this))
		return "exit";


	CMenu *menu = (CMenu *)(m_MainPage.m_Widgets[0]);

	switch(menu->m_Selected)
	{
		case 0:
			return "playgame";
		case 1:
			return "gametypemenu";
		case 2:
			if(m_GameType == JoinNetwork)
			{
				showMessageBox(_("The track must be selected on the server"));
				return "mainmenu";
			}
			return "trackmenu";
		case 3:
			return "playersmenu";
		case 4:
			return "replaymenu";
		case 5:
			return "optionsmenu";
		case 6:
			return "creditsmenu";
		case 7:
			return "exit";
	}

	return "";
}

CString CGameGUI::viewGameTypeMenu()
{
	m_ChildWidget = &m_GameTypePage;
	if(!m_WinSys->runLoop(this))
		return "mainmenu";


	CMenu *menu = (CMenu *)(m_GameTypePage.m_Widgets[0]);

	//A 'safe' starting point:
	m_GameType = LocalGame;

	bool cancelled = false;
	switch(menu->m_Selected)
	{
		case 0:
			m_GameType = LocalGame;
			break;
		case 1:
			m_HostPort = showInputBox(_("Enter the port number:"), m_HostPort, &cancelled).toInt();
			if(cancelled) break;

		case 3:
			//to kill our own server if we had one
			//(it should not be detected by the broadcast):
			initGameType();

			if(menu->m_Selected == 1)
			{
				m_ServerList = CClientNet::broadcast(m_HostPort);
			}
			else
			{
				CMetaServer meta;
				m_ServerList = meta.getServerList();
			}

			if(m_ServerList.size() == 1)
			{
				m_HostName = m_ServerList[0].hostName;
				m_ServerName = m_ServerList[0].serverName;
				m_GameType = JoinNetwork;

				CString msg;
				msg.format(_("Connected to server \"%s\""), 80, m_ServerName.c_str());
				showMessageBox(msg);
			}
			else if(m_ServerList.size() == 0)
			{
				if(!showYNMessageBox(_("Could not find a server. Do you want to set it manually?")) )
					break;

				m_HostName = showInputBox(_("Enter the host name or IP number of the server:"),
					m_HostName, &cancelled);
				if(cancelled) break;

				m_ServerName = m_HostName + ":" + m_HostPort;
				m_GameType = JoinNetwork;
			}
			else //more than one server in the network
			{
				return "selectservermenu";
			}
			break;

		case 2:
		case 4:
			m_HostName = "localhost";
			m_HostPort = showInputBox(_("Enter the port number:"), m_HostPort, &cancelled).toInt();
			if(cancelled) break;
			m_ServerName = showInputBox(_("Enter a name for the server:"), "UStunts", &cancelled);
			if(cancelled) break;
			m_MinPlayers = showInputBox(_("Minimum number of players to wait for (including AI players):"),
				m_MinPlayers, &cancelled).toInt();
			if(cancelled) break;
			m_GameType = NewNetwork;
			m_OnlineServer = (menu->m_Selected == 4);

			break;
	}

	initGameType();

	return "mainmenu";
}

CString CGameGUI::viewSelectServerMenu()
{
	m_ChildWidget = &m_SelectServerPage;
	if(!m_WinSys->runLoop(this))
	{
		m_GameType = LocalGame;
		initGameType();
		return "mainmenu";
	}

	CMenu *menu = (CMenu *)(m_SelectServerPage.m_Widgets[0]);

	m_HostName = m_ServerList[menu->m_Selected].hostName;
	m_HostPort = m_ServerList[menu->m_Selected].port;
	m_ServerName = m_ServerList[menu->m_Selected].serverName;
	m_GameType = JoinNetwork;
	initGameType();

	return "mainmenu";
}

CString CGameGUI::viewTrackMenu()
{
	//1st place hiscore entry
	{
		CHiscoreFile f(m_TrackFile);
		CHiscore hiscore = f.getEntries();

		CLabel *label = (CLabel *)(m_TrackPage.m_Widgets[0]);
		if(hiscore.size() == 0)
			{label->m_Text = _("No best time available");}
		else
		{
			SHiscoreEntry best = hiscore[0];
			label->m_Text = CString(_("Best time: ")) +
				CString().fromTime(best.time) + "   " + best.name + " (" + best.carname + ")";
		}
	}

	m_ChildWidget = &m_TrackPage;
	if(!m_WinSys->runLoop(this))
	{
		if(m_GameType == NewNetwork && m_Server != NULL)
			m_Server->set("track", m_TrackFile);
	
		initGameType(); //sets the track in local game mode

		return "mainmenu";
	}

	CMenu *menu = (CMenu *)(m_TrackPage.m_Widgets[1]);

	switch(menu->m_Selected)
	{
	case 0: //Load track
		return "loadtrackmenu";
	case 1: //Edit track
		return "edittrack";
	case 2: //View replay
	{
		int pos = m_TrackFile.inStr(".track");
		m_ReplayFile = m_TrackFile.mid(0, pos) + ".repl";

		if(!dataFileExists(m_ReplayFile))
			{showMessageBox(_("The best time replay for this track does not exist"));}
		else
			{viewReplay();}

		return "trackmenu";
	}
	case 3: //Return to main menu
	default:
	{
		if(m_GameType == NewNetwork && m_Server != NULL)
			m_Server->set("track", m_TrackFile);
	
		initGameType(); //sets the track in local game mode

		return "mainmenu";
	}
	}

	//Never happens
	return "trackmenu";
}

CString CGameGUI::editTrack()
{
	theTrackDocument = new CTrackDocument(m_TrackFile);

	//The class that is used here, is defined in the
	//top part of this file.
	CTEGUI *gui = new CTEGUI_InGameGUI(m_WinSys, m_SoundSystem);

	leave2DMode();
	gui->start();
	enter2DMode();

	if(dataFileExists(theTrackDocument->m_Trackname))
		m_TrackFile = theTrackDocument->m_Trackname;

	delete gui;
	delete theTrackDocument;
	theTrackDocument = NULL;

	return "trackmenu";
}

CString CGameGUI::viewLoadTrackMenu()
{
	m_ChildWidget = &m_LoadTrackPage;
	if(!m_WinSys->runLoop(this))
		return "trackmenu";

	CMenu *menu = (CMenu *)(m_LoadTrackPage.m_Widgets[0]);

	m_TrackFile = CString("tracks/") + menu->m_Lines[menu->m_Selected];

	return "trackmenu";
}

CString CGameGUI::viewReplayMenu()
{
	m_ChildWidget = &m_ReplayPage;
	if(!m_WinSys->runLoop(this))
		return "mainmenu";

	CMenu *menu = (CMenu *)(m_ReplayPage.m_Widgets[0]);

	m_ReplayFile = CString("replays/") + menu->m_Lines[menu->m_Selected];

	return "viewreplay";
}

CString CGameGUI::viewPlayersMenu()
{
	m_ChildWidget = &m_PlayersPage;
	if(!m_WinSys->runLoop(this)) //cancelling gives false
		return "mainmenu";

	//The rest is executed when not cancelling

	CMenu *menu = (CMenu *)(m_PlayersPage.m_Widgets[0]);

	//menu->m_Lines[menu->m_Selected]
	unsigned int tomain = menu->m_Lines.size()-1;
	unsigned int addplayer = menu->m_Lines.size()-2;
	unsigned int selected = menu->m_Selected;

	if(selected == tomain) return "mainmenu";

	if(selected == addplayer)
	{
		SPlayerDescr newpl;
		newpl.name = "AI Player";
		newpl.isHuman = false;
		newpl.automaticGearbox = true;
		for(unsigned int i=0; i < m_CarFiles.size(); i++)
			if(m_CarFiles[i].filename == "cars/ferrarispider.conf")
			{
				newpl.carIndex = i;
				newpl.carColor = m_CarFiles[i].defaultColor;
				break;
			}
		m_PlayerDescr.push_back(newpl);
	}

	m_SelectedPlayer = selected;
	return "playermenu";
}

CString CGameGUI::viewPlayerMenu()
{
	m_CarViewer->loadCar(
		m_CarFiles[m_PlayerDescr[m_SelectedPlayer].carIndex].filename,
		m_PlayerDescr[m_SelectedPlayer].carColor);

	m_ChildWidget = &m_PlayerPage;
	if(!m_WinSys->runLoop(this)) //cancelling returns false
		return "playersmenu";

	//the rest is done when there is no cancelling

	CMenu *menu = (CMenu *)(m_PlayerPage.m_Widgets[1]);

	switch(menu->m_Selected)
	{
	case 0: //Player type
		m_PlayerDescr[m_SelectedPlayer].isHuman ^= true;
		return "playermenu";
	case 1: //name
		{
			CString original = m_PlayerDescr[m_SelectedPlayer].name;
			bool cancelled = false;
			m_PlayerDescr[m_SelectedPlayer].name =
				showInputBox(_("Enter the name:"), "", &cancelled);
			if(cancelled) m_PlayerDescr[m_SelectedPlayer].name = original;
		}
		return "playermenu";
	case 2: //car
		return "carmenu";
	case 3: //Gearbox
		m_PlayerDescr[m_SelectedPlayer].automaticGearbox ^= true;
		return "playermenu";
	case 4: //car color
		//m_PlayerDescr[m_SelectedPlayer].carColor = showInputBox(_("Enter the car color:"),
		//	m_PlayerDescr[m_SelectedPlayer].carColor).toVector();
		m_PlayerDescr[m_SelectedPlayer].carColor = showColorSelect(_("Car color:"),
			m_PlayerDescr[m_SelectedPlayer].carColor);
		return "playermenu";
	case 5: //delete
		if(m_PlayerDescr.size() == 1)
		{
			showMessageBox(_("Can't delete: there needs to be at least one player"));
			return "playermenu";
		}
		if(showYNMessageBox(_("Delete this player?")) )
		{
			m_PlayerDescr.erase(m_PlayerDescr.begin() + m_SelectedPlayer);
			m_SelectedPlayer = 0;
			return "playersmenu";
		}
		return "playermenu";
	case 6: //return
		return "playersmenu";
	}

	return "playersmenu";
}

CString CGameGUI::viewCarMenu()
{
	m_ChildWidget = &m_CarPage;
	if(m_WinSys->runLoop(this))
	{
		CMenu *menu = (CMenu *)(m_CarPage.m_Widgets[0]);
		m_PlayerDescr[m_SelectedPlayer].carIndex = menu->m_Selected;
		m_PlayerDescr[m_SelectedPlayer].carColor = m_CarFiles[menu->m_Selected].defaultColor;
	}

	return "playermenu";
}

CString CGameGUI::viewOptionsMenu()
{
	CMenu *menu = (CMenu *)(m_OptionsPage.m_Widgets[0]);
	if(m_SelectedOptionsSection != "")
	{
		m_OptionsPage.m_Title.format(_("Settings: %s"), 80, m_SelectedOptionsSection.c_str());

		vector<CString> options = theMainConfig->getFieldsInSection(m_SelectedOptionsSection);
		for(unsigned int i=0; i < options.size(); i++)
		{
			CString value = theMainConfig->getValue(m_SelectedOptionsSection, options[i]);

			//Force them to be the same length, for alignment
			unsigned int vlen = value.length(), olen = options[i].length();
			if(vlen > olen)
			{
				options[i] = CString().forceLength(vlen - olen) + options[i];
			}
			else
			{
				value = value.forceLength(olen);
			}

			options[i] = options[i] + " = " + value;
		}

		menu->m_Lines = options;
		menu->m_Lines.push_back(_("Ready"));
		menu->m_AlignLeft = false;
	}

	m_ChildWidget = &m_OptionsPage;
	if(!m_WinSys->runLoop(this)) //cancelling returns false
	{
		if(m_SelectedOptionsSection == "")
		{
			if(m_OptionsAreChanged)
			{
				bool cancelled = false;
				bool ans = showYNMessageBox(
					_("Some settings are changed. Do you wish to save the settings?"), &cancelled);
				if(cancelled) return "optionsmenu";
				if(ans)
				{
					//save options
					if(!theMainConfig->saveFile())
					{
						showMessageBox(_("Saving failed"));
					}
					else
					{
						m_OptionsAreChanged = false;
					}
				}
			}

			return "mainmenu";
		}

		m_SelectedOptionsSection = "";
		menu->m_Selected = 0; //scroll back to the start
		return "optionsmenu";
	}

	if(m_SelectedOptionsSection == "")
	{
		if(menu->m_Selected == menu->m_Lines.size()-1)
		{
			if(m_OptionsAreChanged)
			{
				bool cancelled = false;
				bool ans = showYNMessageBox(
					_("Some settings are changed. Do you wish to save the settings?"), &cancelled);
				if(cancelled) return "optionsmenu";
				if(ans)
				{
					//save options
					if(!theMainConfig->saveFile())
					{
						showMessageBox(_("Saving failed"));
					}
					else
					{
						m_OptionsAreChanged = false;
					}
				}
			}

			return "mainmenu";
		}

		m_SelectedOptionsSection = menu->m_Lines[menu->m_Selected];
		return "optionsmenu";
	}
	
	if(menu->m_Selected == menu->m_Lines.size()-1)
	{
		m_SelectedOptionsSection = "";
		menu->m_Selected = 0; //scroll back to the start
		return "optionsmenu";
	}

	CString option = theMainConfig->getFieldsInSection(m_SelectedOptionsSection)[menu->m_Selected];
	CString oldValue = theMainConfig->getValue(m_SelectedOptionsSection, option);
	CString metaData = theMainConfig->getMetaData(m_SelectedOptionsSection, option);
	bool cancelled = false;
	CString newValue = showSettingBox(option, oldValue, metaData, &cancelled);
	if(!cancelled)
	{
		theMainConfig->setValue(m_SelectedOptionsSection, option, newValue);

		if(!reloadConfiguration())
		{
			showMessageBox(_("Applying the new value failed"));
			theMainConfig->setValue(m_SelectedOptionsSection, option, oldValue);
		}
		else
		{
			m_OptionsAreChanged = true;
		}
	}

	return "optionsmenu";
}

CString CGameGUI::viewCreditsMenu()
{
	m_ChildWidget = &m_CreditsPage;
	m_WinSys->runLoop(this);
	
	return "mainmenu";
}

CString CGameGUI::viewHiscore()
{
	CHiscore hiscore = m_GameCore->getHiscore();
	m_ReplayFile = m_GameCore->getReplayFile();

	//add hiscore data to the menu
	CMenu *names = (CMenu *)m_HiscorePage.m_Widgets[0];
	CMenu *cars = (CMenu *)m_HiscorePage.m_Widgets[1];
	CMenu *times = (CMenu *)m_HiscorePage.m_Widgets[2];
	names->m_Lines.clear();
	cars->m_Lines.clear();
	times->m_Lines.clear();
	for(unsigned int i=0; i < hiscore.size(); i++)
	{
		CString name =
			CString().format("%d ", 8, i+1) +
			hiscore[i].name + "  ....................";
		CString car = CString(" ") + hiscore[i].carname + "  ....................";
		CString time = CString().fromTime(hiscore[i].time);

		if(hiscore[i].isNew) time += _("  (this race)");

		names->m_Lines.push_back(name);
		cars->m_Lines.push_back(car);
		times->m_Lines.push_back(time);
	}

	m_ChildWidget = &m_HiscorePage;
	if(!m_WinSys->runLoop(this))
		return "mainmenu";


	CMenu *menu = (CMenu *)m_HiscorePage.m_Widgets[3];

	switch(menu->m_Selected)
	{
		case 0:
			viewReplay();
			return "hiscore";
			break;
		case 1:
			{
			bool cancelled = false;
			CString newFN = showInputBox(_("Enter the replay filename:"), "abc.repl", &cancelled);
			if(cancelled) return "hiscore"; //and don't save

			//check extension
			if(newFN.mid(newFN.length()-5) != ".repl")
				newFN += ".repl";

			//The real datadir filename
			CString fn = "replays/" + newFN;

			//Check if we overwrite something
			if(dataFileExists(fn, true)) //search only locally
			{
				CString q;
				q.format(_("File %s already exists. Overwrite?"), 120, newFN.c_str());
				if(!showYNMessageBox(q)) return "hiscore"; //and don't save
			}

			//Then the final copying
			copyDataFile(m_ReplayFile, fn);

			return "hiscore";
			}
			break;
		case 2:
			return "playgame";
			break;
		case 3:
			return "mainmenu";
			break;
	}

	return "";
}

/*
----------------------------------------
   THE NON-MENU FUNCTIONS
----------------------------------------
*/

void CGameGUI::initGameType(bool keepServerAlive, bool keepReplayFile)
{
	//Use this as a safe starting point
	m_GameCore->initLocalGame(m_TrackFile);

	//Start or stop the server
	if(m_GameType == NewNetwork)
	{
		if(m_Server == NULL)
		{
			//start server if needed
			m_Server = new CUSServer(m_HostPort, m_ServerName);
		}

		if(!m_OnlineServer)
			{m_Server->offline();}

		//update server settings
		m_Server->set("port", m_HostPort);
		m_Server->set("serverName", m_ServerName);
		m_Server->set("minPlayers", m_MinPlayers);
		m_Server->set("track", m_TrackFile);
		m_Server->set("saveHiscore", "false"); //else, they would be saved twice on this computer
		sleep(1); //give the server some time to start

		if(m_OnlineServer)
			{m_Server->online();}
	}
	else
	{
		//stop server process if needed
		if(!keepServerAlive && m_Server != NULL)
			{delete m_Server; m_Server = NULL;}
	}


	if(m_GameType == ViewReplay)
	{
		m_GameCore->initReplayGame(m_ReplayFile);
	}
	else if(m_GameType == JoinNetwork || m_GameType == NewNetwork)
	{
		//connect to the server
		if(!(m_GameCore->initClientGame(m_HostName, m_HostPort, keepReplayFile)) )
		{
			showMessageBox(_("Connecting to the server failed"));
			m_GameType = LocalGame;
		}
	}
}

CString CGameGUI::viewReplay()
{
	eGameType oldType = m_GameType;

	m_GameType = ViewReplay;
	initGameType(true, true);

	playGame(); //ignore its return value: we need to return here

	m_GameType = oldType;
	initGameType(true, true);

	return "mainmenu";
}

CString CGameGUI::playGame()
{
	if(!load())
	{
		unload();
		return "mainmenu";
	}

	m_WinSys->showMouseCursor(false);
	leave2DMode();

	_USGameCore = m_GameCore;
	m_WinSys->runLoop(game_mainloop, true); //true: swap buffers

	enter2DMode();
	m_WinSys->showMouseCursor(true);

	unload();

	return "hiscore";
}

bool CGameGUI::load()
{
	m_ChildWidget = &m_LoadingPage;
	onResize(0, 0, m_W, m_H);
	onRedraw();
	m_WinSys->swapBuffers();

	if(m_GameType != ViewReplay) //don't add players for a replay
	{
		//add players
		unsigned int numHumanPlayers = 0;
		for(unsigned int i=0; i < m_PlayerDescr.size(); i++)
		{
			if(m_GameType == NewNetwork && !(m_PlayerDescr[i].isHuman))
				continue; //AI players will be added on the server side

			//TODO: maybe use CObjectChoice class for m_PlayerDescr array
			CPlayer *p;
			CObjectChoice choice;
			choice.m_Filename = m_CarFiles[m_PlayerDescr[i].carIndex].filename;
			choice.m_PlayerName = m_PlayerDescr[i].name;
			{
				SParameter p;
				p.name = "color";
				p.value = m_PlayerDescr[i].carColor;
				choice.m_Parameters.push_back(p);
			}

			if(m_PlayerDescr[i].isHuman)
			{
				CHumanPlayer *hp = new CHumanPlayer((CGameWinSystem *)m_WinSys, numHumanPlayers);
				hp->m_AutomaticGear = m_PlayerDescr[i].automaticGearbox;
				p = hp;
			}
			else
			{
				p = new CAIPlayerCar();
			}

			if(!m_GameCore->addPlayer(p, choice))
			{
				showMessageBox(
					CString().format(_("Player %s was refused"), 80,
					m_PlayerDescr[i].name.c_str())
					);
				onRedraw();
				m_WinSys->swapBuffers();

				delete p;
				continue;
			}

			if(m_PlayerDescr[i].isHuman) //set a camera to this player:
			{
				m_GameCore->addCamera(p->m_MovingObjectId);
				numHumanPlayers++;
			}

			m_Players.push_back(p);
		}

		//add a camera for an AI player if there are no human players:
		if(numHumanPlayers == 0 || m_GameType == ViewReplay)
		{
			m_GameCore->addCamera(0); //ASSUMES that 0 is the first ID (which is the case)
		}
	}

	//add a camera for a replay game
	if(m_GameType == ViewReplay)
	{
		m_GameCore->addCamera(0); //ASSUMES that 0 is the first ID (which is the case)
	}

	if(m_GameType == NewNetwork)
	{
		//clear the AI list
		m_Server->clearai();

		//In a new network game, place the AI's on the server side
		for(unsigned int i=0; i < m_PlayerDescr.size(); i++)
			if(!(m_PlayerDescr[i].isHuman))
				m_Server->addai(m_PlayerDescr[i].name, m_CarFiles[m_PlayerDescr[i].carIndex].filename);

		sleep(1); //To give messages some time to arive on server (TODO: remove when no longer needed)

		//start the game on the server side
		m_Server->start();
	}

	_LoadingPage = &m_LoadingPage;
	
	if(!m_GameCore->readyAndLoad(loadingCallback))
		return false; //Loading failed, or was cancelled

	m_GameCore->setStartTime();
	return true;
}

void CGameGUI::unload()
{
	m_GameCore->stopGame();

	if(m_Server != NULL)
		m_Server->stop();

	for(unsigned int i=0; i<m_Players.size(); i++)
		delete m_Players[i];
	m_Players.clear();

	if(m_Server != NULL)
	{
		//clear the AI list
		m_Server->clearai();
	}

	//do not yet unload the server: we might want to play again
}
