/***************************************************************************
                          playercontrol.cpp  -  Manage the players
                             -------------------
    begin                : do sep 11 2003
    copyright            : (C) 2003 by CJP
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

#include "playercontrol.h"

CPlayerControl::CPlayerControl(CWorld *w)
{
	m_World = w;
}

CPlayerControl::~CPlayerControl()
{
}

int CPlayerControl::addPlayer(CObjectChoice choice)
{
	//Default behaviour: always accept a player
	m_LocalChoices.push_back(choice);
	return m_LocalChoices.size()-1;
}

bool CPlayerControl::loadObjects()
{
	//Temporary solution: nr of objs put in string
	return m_World->loadMovObjs((CString)( (int)m_LocalChoices.size() ));
}
