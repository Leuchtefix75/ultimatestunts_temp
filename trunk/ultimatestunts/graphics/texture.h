/***************************************************************************
                          texture.h  -  A texture class
                             -------------------
    begin                : Tue Jun 18 2002
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
#ifndef TEXTURE_H
#define TEXTURE_H
#include "image.h"
#include "vector.h"

#include "cstring.h"

class CTexture
{
public:
	CTexture();
	void setTextureSmooth(bool texture_smooth)
		{m_TextureSmooth = texture_smooth;}

	virtual bool loadFromFile(CString filename, int xs, int ys);
	virtual void unload();

	int getSizeX(int i) const;
	int getSizeY(int i) const;
	CVector getColor() const;

  void draw(int lod) const;

protected:
  RGBImageRec *scaleImage(RGBImageRec *in, int xs, int ys);

	bool m_TextureSmooth;

	unsigned int m_Texture;
	unsigned int m_Texture2;
	unsigned int m_Texture3;
	unsigned int m_Texture4;

	int sizex,sizey,
			sizex2,sizey2,
			sizex3,sizey3,
			sizex4,sizey4;

	CVector m_Color;
};

#endif