/***************************************************************************
                          simulation.cpp  -  Base-class for simulations
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

#include <stdio.h>
#include "simulation.h"

CSimulation::CSimulation(CWorld *w)
{
  m_World = w;
}

CSimulation::~CSimulation(){
}

int CSimulation::addPlayer(CObjectChoice choice)
{
	//Default behaviour: always accept a player
	m_LocalChoices.push_back(choice);
	return m_LocalChoices.size()-1;
}

bool CSimulation::loadObjects()
{
	//Temporary solution: nr of objs put in string
	return m_World->loadMovObjs((CString)( (int)m_LocalChoices.size() ));
}