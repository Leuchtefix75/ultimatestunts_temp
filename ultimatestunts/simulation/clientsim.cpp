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

#include <cstdio>

#include "clientsim.h"
#include "textmessage.h"
#include "movingobject.h"

CClientSim::CClientSim(CGameCore *gc, CClientNet *net)
{
	m_GameCore = gc;
	m_Net = net;
	
	m_PreviousTime = m_Timer.getTime();
}

CClientSim::~CClientSim()
{
}

CString CClientSim::getTrackname()
{
	m_Net->sendTextMessage("READY");

	while(true)
	{
		CMessageBuffer *buf = m_Net->receiveExpectedData(CMessageBuffer::textMessage, 10000);
		if(buf != NULL)
		{
			CTextMessage tm;
			tm.setBuffer(*buf);

			if(buf->getAC() != 0)
				m_Net->sendConfirmation(*buf, 0);

			delete buf;

			if(tm.m_Message.mid(0,6) == "TRACK=")
				return tm.m_Message.mid(6);
		}
	}

	return "tracks/default.track"; //we'll never reach this anyway
}

bool CClientSim::update()
{
	//the moving objects:
	vector<CDataObject *> objs = theWorld->getObjectArray(CDataObject::eMovingObject);

	//if some amount of time has passed:
	float now = m_Timer.getTime();
	if(now > m_PreviousTime + 0.04) //25 times per second
	{
		m_PreviousTime = now;
		
		//send input info
		for(unsigned int j=0; j < objs.size(); j++)
		{
			CMovingObject *mo = (CMovingObject *)objs[j];

			if( !m_GameCore->isLocalPlayer(mo->getMovObjID()) ) continue; //only local players

			CMessageBuffer b = mo->m_InputData->getBuffer();
			m_Net->sendData(b);
		}
	}

	//receive object info
	//the receiveData function was already called by the gamecore
	for(unsigned int i=0; i < m_Net->m_ReceiveBuffer.size(); i++)
	{
		CMessageBuffer &buf = m_Net->m_ReceiveBuffer[i];
		switch(buf.getType())
		{
		case CMessageBuffer::car:
		case CMessageBuffer::movingObject:
		{
			CBinBuffer data = buf.getData();
			unsigned int pos = 0;
			Uint8 ID = data.getUint8(pos);

			//find the corresponding moving object
			CMovingObject *mo = NULL;
			//first candidate
			if(ID < objs.size())
			{
				CMovingObject *tmp = (CMovingObject *)objs[ID];
				if(tmp->getMovObjID() == ID)
					mo = tmp;
			}
			//all other objects
			if(mo == NULL)
				for(unsigned int j=0; j < objs.size(); j++)
				{
					CMovingObject *tmp = (CMovingObject *)objs[j];
					if(tmp->getMovObjID() == ID)
						mo = tmp;
				}

			if(mo == NULL) break; //not found

			mo->setBuffer(buf);
		}
			break;
		default:
			break;
		}
		//TODO: maybe delete the used messages (maybe not necessary)
	}

	return true;
}
