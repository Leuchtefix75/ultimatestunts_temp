/***************************************************************************
                          clientsim.cpp  -  Client-side networked simulation
                             -------------------
    begin                : di jan 14 2003
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

#include "physics.h"
//#include "approximation.h"
#include "clientsim.h"

CClientSim::CClientSim(CWorld *w, CString HostName, int UDPPort) : CSimulation(w)
{
	m_Network = new CClientNet(HostName, UDPPort);

	//TODO: check connection speed

	//if(slownet)
		{m_SubSim = new CPhysics(w);}
	//else
	//	{m_SubSim = new CApproximation(w);}
}

CClientSim::~CClientSim()
{
	delete m_SubSim;
}

CString CClientSim::getTrackname()
{
	return "tracks/default.track"; //TODO: really find out what track should be used
}

int CClientSim::addPlayer(CObjectChoice choice)
{
	//TODO: Register player via network and check if the player is accepted

	return CSimulation::addPlayer(choice);
}

bool CClientSim::loadObjects()
{
	//TODO: Wait for server information and use this to load all objects

	return CSimulation::loadObjects();
}

bool CClientSim::update()
{
	//TODO: check network for new data, and update the world
	return m_SubSim->update();
}