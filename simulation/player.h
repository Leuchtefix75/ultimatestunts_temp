/***************************************************************************
                          player.h  -  Base-class for players
                             -------------------
    begin                : Wed Dec 4 2002
    copyright            : (C) 2002 by CJP
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

#ifndef PLAYER_H
#define PLAYER_H

#include "world.h"
#include "message.h"
#include "cstring.h"

/**
  *@author CJP
  */

class CPlayer {
public: 
	CPlayer();
	virtual ~CPlayer();

	virtual bool update()
		{return true;} //default empty implementation

	int m_MovingObjectId;
	int m_PlayerId;

	CString m_Name;
protected:
	const CWorld *m_World;

	int setAutomaticGear(float &gas, float &brake);
};

#endif
