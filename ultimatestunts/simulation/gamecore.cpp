/***************************************************************************
                          gamecore.cpp  -  The core of the game
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

#include "gamecore.h"

CGameCore::CGameCore()
{
	m_World = new CWorld();
	m_PCtrl = NULL;
	m_ClientNet = NULL;
	initLocalGame(""); //just to get it into some state
}

CGameCore::~CGameCore()
{
	unloadGame();

	if(m_ClientNet!=NULL)
		delete m_ClientNet;

	delete m_World;
}

void CGameCore::initLocalGame(const CString &trackfile)
{
	unloadGame();

	//delete client network of previous game sessions
	if(m_ClientNet!=NULL)
		delete m_ClientNet;
	m_ClientNet = NULL;
	resetGame();
	m_TrackFile = trackfile;
}

void CGameCore::initClientGame(const CString &host, unsigned int port)
{
	unloadGame();

	//delete old clientnet only if it's different
	if(m_ClientNet != NULL && !(m_ClientNet->getHost() == host && m_ClientNet->getPort() == port))
	{
		delete m_ClientNet;
		m_ClientNet = NULL;
	}

	//new clientnet if we don't have one
	if(m_ClientNet == NULL)
		{m_ClientNet = new CClientNet(host, port);}

	resetGame();
}

bool CGameCore::addPlayer(CPlayer *p, CString name, CObjectChoice choice)
{
	if(m_PCtrl == NULL) return false;

	int id = m_PCtrl->addPlayer(choice);
	p->m_MovingObjectId = id;
	p->m_PlayerId = 0;
	if(id < 0)
		return false; //sim doesn't accept player

	m_Players.push_back(p);
	return true;
}

void CGameCore::readyAndLoad()
{
	if(	m_ClientNet != NULL)
	{
		//TODO: send ready, and wait for start
		CClientSim *csim = (CClientSim *)(m_Simulations[0]);
		m_TrackFile = csim->getTrackname();
	}
	
	loadTrackData();
	loadMovObjData();
}

bool CGameCore::update() //true = continue false = leave
{
	for(unsigned int i=0; i<m_Players.size(); i++)
		m_Players[i]->update(); //Makes moving decisions

	
	bool retval = true;
	for(unsigned int i=0; i<m_Simulations.size(); i++)
	{
		retval = retval && m_Simulations[i]->update(); //Modifies world object
	}

	return retval;
}

void CGameCore::resetGame()
{
	unloadGame();

	if(m_ClientNet == NULL) //local game
	{
		m_PCtrl = new CPlayerControl;
		m_Simulations.push_back(new CRuleControl(m_World));
		m_Simulations.push_back(new CPhysics(theMainConfig, m_World));
	}
	else
	{
		m_PCtrl = new CClientPlayerControl(m_ClientNet);
		m_Simulations.push_back(new CClientSim(m_ClientNet, m_World));
		m_Simulations.push_back(new CPhysics(theMainConfig, m_World));
	}
}

void CGameCore::unloadGame()
{
	for(unsigned int i=0; i<m_Simulations.size(); i++)
		delete m_Simulations[i];
	m_Simulations.clear();

	m_Players.clear();

	if(m_PCtrl!=NULL)
		delete m_PCtrl;
	m_PCtrl = NULL;

	unloadData();
}

void CGameCore::loadTrackData()
{
	printf("---World track data\n");
	m_World->loadObject(m_TrackFile, CParamList(), CDataObject::eTrack);
}

void CGameCore::loadMovObjData()
{
	printf("---World object data\n");
	if(!m_PCtrl->loadObjects()) //loads them into the world
		{printf("Error while loading moving objects\n");}
}

void CGameCore::unloadData()
{
	printf("---World data\n");
	m_World->unloadAll();
}
