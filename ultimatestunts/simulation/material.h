/***************************************************************************
                          material.h  -  Friction coefficients etc.
                             -------------------
    begin                : ma jan 27 2003
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

#ifndef MATERIAL_H
#define MATERIAL_H

#include "cstring.h"

/**
  *@author CJP
  */

class CMaterial {
public:
	virtual ~CMaterial(){;}

	virtual bool loadFromFile(CString filename, int mul)
		{m_Filename = filename; m_Mul = mul; return true;}

	CString getFilename()
		{return m_Filename;}

	int getTextureMul()
		{return m_Mul;}

protected:
	CString m_Filename; //Texture filename
	int m_Mul; //texture size multiplier
};

#endif