/***************************************************************************
                          movingobject.h  -  description
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

#ifndef MOVINGOBJECT_H
#define MOVINGOBJECT_H


/**
  *@author CJP
  */

#include "dynamicobject.h"

class CMovingObject : public CDynamicObject {
public: 
	CMovingObject();
	~CMovingObject();

	virtual CMessageBuffer::eMessageType getType() const {return CMessageBuffer::movingObject;}
};

#endif
