/***************************************************************************
                          player.cpp  -  description
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

#include "player.h"

CPlayer::CPlayer()
{
  //These have not been set:
  m_MovingObjectId = -1;
  m_PlayerId = -1;
}

CPlayer::~CPlayer(){
}

void CPlayer::giveWorld(const CWorld *w)
{m_World = w;}

const CMessage *CPlayer::getInputData()
{
  return NULL; //Base-class doesn't generate input data
}

bool CPlayer::Update()
{}