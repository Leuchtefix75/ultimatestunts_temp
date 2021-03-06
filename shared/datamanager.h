/***************************************************************************
                          datamanager.h  -  Managing loading/unloading data objects
                             -------------------
    begin                : wo dec 1 2004
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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "dataobject.h"

/**
  *@author CJP
  */

class CDataManager {
public: 
	CDataManager();
	virtual ~CDataManager();

	/*
	loads a file if it isn't already loaded
	returns the ID
	or -1 on failure

	ID's count up from 0 (every type counts separately)
	*/
	int loadObject(const CString &filename, const CParamList plist, CDataObject::eDataType type=CDataObject::eNone);

	//returns the object with the ID
	CDataObject *getObject(CDataObject::eDataType type, unsigned int ID);
	const CDataObject *getObject(CDataObject::eDataType type, unsigned int ID) const;
	unsigned int getNumObjects(CDataObject::eDataType type) const
		{return m_Objects[type].size();}

	vector<CDataObject *> getObjectArray(CDataObject::eDataType type);
	vector<const CDataObject *> getObjectArray(CDataObject::eDataType type) const;

	//Get array of objects from string containing integer IDs
	vector<CDataObject *> getSubset(CDataObject::eDataType type, const CString &subset);

	//Input: string with filenames and parameters
	//Output: string with integers that can be given to getSubset
	CString loadFilesFromString(CDataObject::eDataType type, const CString &files);

	//This invalidates all IDs >= i:
	virtual void unloadObject(unsigned int type, unsigned int i);

	virtual void unloadAll(CDataObject::eDataType type=CDataObject::eNone);

protected:
	virtual CDataObject *createObject(const CString &filename, const CParamList &plist, CDataObject::eDataType type);

	int findObject(const CString &filename, const CParamList plist, CDataObject::eDataType type);
	
	vector<vector<CDataObject *> > m_Objects; //two dimensional array
};

#endif
