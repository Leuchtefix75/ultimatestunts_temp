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

/**
  *@author CJP
  */

struct STile {
	int m_Model;
	int m_Z, m_R; //height, orientation. 0 <= m_R <= 3

	bool m_isStart, m_isFinish;
	float m_Time;
};

class CTrack : public CDataObject  {
public: 
	CTrack(CDataManager *manager);
	virtual ~CTrack();

	virtual bool load(const CString &filename, const CParamList &list);
	virtual void unload();

	vector<STile> m_Track; //refer to tile model elements in the manager object
	int m_L, m_W, m_H;

	CString m_BackgroundFilename;
	CString m_EnvMapFilename;
};

#endif