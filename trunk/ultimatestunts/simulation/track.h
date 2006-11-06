/***************************************************************************
                          track.h  -  A racing track
                             -------------------
    begin                : di dec 7 2004
    copyright            : (C) 2004 by CJP
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

#ifndef TRACK_H
#define TRACK_H

#include <vector> //STL vector template
namespace std {}
using namespace std;

#include "dataobject.h"
#include "datafile.h"
#include "vector.h"

/**
  *@author CJP
  */

struct STile {
	int m_Model;
	int m_Z, m_R; //height, orientation. 0 <= m_R <= 3
};

class CTrack : public CDataObject  {
public: 
	CTrack(CDataManager *manager);
	virtual ~CTrack();

	virtual bool load(const CString &filename, const CParamList &list);
	virtual void unload();

	vector<STile> m_Track; //refer to tile model elements in the manager object
	int m_L, m_W, m_H;

	//int m_FinishRouteCounter; //same meaning as in the tile data
	//float m_FinishTime;

	//New rule data format:
	class CCheckpoint
	{
	public:
		int x, y, z;

		bool operator==(const CCheckpoint &val) const
			{return x==val.x && y==val.y && z==val.z;}
	};
	class CRoute : public vector<CCheckpoint>
	{
	public:
		unsigned int startRoute, startTile;
		unsigned int finishRoute, finishTile;
	};
	vector<CRoute> m_Routes;

	CString m_BackgroundFilename;
	CString m_EnvMapFilename;
	CVector m_LightDirection;
	CVector m_LightColor;
	CVector m_AmbientColor;

private:
	bool findBeginTag(CDataFile &f);
};

#endif