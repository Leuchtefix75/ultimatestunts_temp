/***************************************************************************
                          gamecore.h  -  The core of the game
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

#ifndef GAMECORE_H
#define GAMECORE_H

#include <vector>
namespace std {}
using namespace std;

//Frequently used
#include "cstring.h"
#include "lconfig.h"
#include "usmacros.h"

//Simulation stuff
#include "playercontrol.h"
#include "clientplayercontrol.h"
#include "rulecontrol.h"
#include "world.h"

//Player stuff
#include "objectchoice.h"
#include "player.h"

//Other things
#include "filecontrol.h"
#include "hiscorefile.h"

/**
  *@author CJP
  */

class CGameCore {
public: 
	CGameCore();
	virtual ~CGameCore();

	//step 1: initialise network connection etc.
	//the last function call defines the current state
	void initLocalGame(const CString &trackfile);
	void initClientGame(const CString &host, unsigned int port);

	//step 2: add players
	bool addPlayer(CPlayer *p, CObjectChoice choice);

	//step 3: wait for the start signal and load everything
	virtual void readyAndLoad();

	//step 3: Play
	virtual bool update(); //true = continue false = leave

	//step 4: Get hiscore+replay data and stop game (go back to step 1 or 2)
	//this does not undo step 1
	//to stop a network connection, init a local game
	void stopGame();

	//step 5: Get result information of last game (hiscore, replay)
	vector<SHiscoreEntry> getHiscore();

	//some tools
	//TODO: place these e.g. in CWorld
	bool isLocalPlayer(unsigned int ID);
protected:
	CWorld *m_World;
	vector<CPlayer *> m_Players;
	CPlayerControl *m_PCtrl;
	vector<CSimulation *> m_Simulations;
	CClientNet *m_ClientNet;
	CString m_TrackFile;

	vector<SHiscoreEntry> m_LastHiscores;

	virtual void loadTrackData();
	virtual void loadMovObjData();

	void collectHiscoreData();

	void resetGame();
	void unloadGame();
	virtual void unloadData();
};

#endif
