/***************************************************************************
                          graphobj.h  -  Graphics geometry object
                             -------------------
    begin                : Tue Apr 23 2002
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
#ifndef GRAPHOBJ_H
#define GRAPHOBJ_H

#include "lodtexture.h"
#include "cstring.h"
#include "cfile.h"

class CGraphObj
{
	public:
		CGraphObj();
		virtual ~CGraphObj();
		virtual bool loadFromFile(CFile *f, CLODTexture **matarray, int lod_offset = 0);
		void unload();

		void draw(int lod) const;
	protected:
		unsigned int m_ObjListRef, m_ObjList1, m_ObjList2, m_ObjList3, m_ObjList4;

		float m_OpacityState;
		CVector m_ColorState;
		void setMaterialColor();
};

#endif
