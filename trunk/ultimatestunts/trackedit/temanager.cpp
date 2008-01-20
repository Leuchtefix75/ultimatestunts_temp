/***************************************************************************
                          temanager.cpp  -  The data manager for the track editor
                             -------------------
    begin                : ma mei 23 2005
    copyright            : (C) 2005 by CJP
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

#include "temanager.h"

#include "edittrack.h"
#include "lodtexture.h"
#include "graphobj.h"

CTEManager::CTEManager(){
}
CTEManager::~CTEManager(){
}

CDataObject *CTEManager::createObject(const CString &filename, const CParamList &plist, CDataObject::eDataType type)
{
	CDataObject *obj = CDataManager::createObject(filename, plist, type);
	if(obj != NULL) return obj;

	if(type == CDataObject::eMaterial)
		return new CLODTexture(this);

	if(type == CDataObject::eTileModel)
		return new CGraphObj(this, CDataObject::eTileModel);

	if(type == CDataObject::eBound)
		return new CGraphObj(this, CDataObject::eBound);

	if(type == CDataObject::eTrack)
		return new CEditTrack(this);

	return NULL;
}

CString CTEManager::getTextureSubset(const CString &textureString)
{
	CString theString = textureString;
	theString.Trim();

	//printf("theString = \"%s\"\n", theString.c_str());

	CString ret;

	while(theString != "")
	{
		CString thisTexture;
		int semicol = theString.inStr(';');
		if(semicol < 0)
		{
			thisTexture = theString;
			theString = "";
		}
		else
		{
			thisTexture = theString.mid(0, semicol);
			theString   = theString.mid(semicol+1);
		}

		thisTexture.Trim();

		//printf("thisTexture = \"%s\"\n", thisTexture.c_str());

		CString texfile;
		CParamList plist;
		int space = thisTexture.inStr(' ');
		if(space < 0)
		{
			texfile = thisTexture;
		}
		else
		{
			texfile = thisTexture.mid(0, space);
			thisTexture = thisTexture.mid(space+1);

			//Texture parameters
			plist = CParamList(thisTexture);
		}

		//printf("texfile = \"%s\"\n", texfile.c_str());

		int texID = loadObject(texfile, plist, CDataObject::eMaterial);

		if(ret == "")
			{ret = CString(texID);}
		else
			{ret += CString(" ") + texID;}
	}

	return ret;
}

void CTEManager::removeUnusedTextures()
{
	unsigned int numTextures = getNumObjects(CDataObject::eMaterial);
	if(numTextures == 0) return;

	vector<bool> isUsed(numTextures, false);

	vector<CDataObject *> tiles = getObjectArray(CDataObject::eTileModel);
	for(unsigned int i=0; i < tiles.size(); i++)
	{
		const CParamList &plist = tiles[i]->getParamList();
		CString subset = plist.getValue("subset", ""); //the texture subset of this tile

		//Find out which textures are used by this tile
		while(!(subset.empty()))
		{
			int ID = 0;
	
			int pos = subset.inStr(' ');
			if(pos < 0)
			{
				ID = subset.toInt();
				subset = "";
			}
			else
			{
				ID = subset.mid(0, pos).toInt();
				subset = subset.mid(pos+1);
			}

			isUsed[ID] = true;
		}

	}

	//Delete all these textures, from high to low to preserve later IDs in this loop
	for(int i=numTextures-1; i >= 0; i--)
	{
		if(!isUsed[i])
			unloadObject(CDataObject::eMaterial, i);
	}

	//Now make the ID translation table
	int decrease = 0;
	vector<int> newID(numTextures, 0);
	for(unsigned int i=0; i < numTextures; i++)
	{
		if(!isUsed[i]) decrease++;
		newID[i] = i - decrease;
	}

	//And translate all subset parameters
	for(unsigned int i=0; i < tiles.size(); i++)
	{
		CParamList plist = tiles[i]->getParamList();

		for(unsigned int j=0; j < plist.size(); j++)
			if(plist[j].name == "subset")
			{
				plist[j].value = CTrack::translateTextureIndices(plist[j].value, newID);
				break;
			}

		//Set new plist by completely reloading the object
		//This also makes sure that all internal pointers etc. are updated
		tiles[i]->load(tiles[i]->getFilename(), plist);
	}

}

