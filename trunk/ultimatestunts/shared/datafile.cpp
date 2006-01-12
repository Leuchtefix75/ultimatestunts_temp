/***************************************************************************
                          datafile.cpp  -  datafile + filecontrol implementations
                             -------------------
    begin                : do mei 1 2003
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

#include <dirent.h>
#include <cstdio>
#include <unistd.h>

#include "filecontrol.h"
#include "datafile.h"


//CFileControl part
CFileControl *theFileControl = NULL;

CFileControl::CFileControl(bool takeover)
{
	if(takeover) theFileControl = this;

	filecontroldatadir = "./data/";
	filecontrolsavedir = "./saveddata/";
}

CFileControl::~CFileControl()
{
	if(theFileControl == this) theFileControl = NULL;
}

void CFileControl::setDataDir(const CString &dir)
{
	filecontroldatadir = dir;
}

void CFileControl::setSaveDir(const CString &dir)
{
	filecontrolsavedir = dir;
}

CString CFileControl::getLongFilename(const CString &shortName)
{
	CString datafile = filecontroldatadir + shortName;
	CString savefile = filecontrolsavedir + shortName;

	//use saved file when it exists
	if(fileExists(savefile))
		return savefile;

	//else, use the original data file
	if(fileExists(datafile))
		return datafile;

	//else, return empty string
	return "";
}


//CDataFile part

CDataFile::CDataFile(CString filename, bool write)
{
	open(filename, write);
}

CDataFile::CDataFile() : CFile()
{}

CDataFile::~CDataFile()
{
	//I gues it's already being closed by CFile
}

bool CDataFile::open(CString filename, bool write)
{
	CString datafile = theFileControl->filecontroldatadir + filename;

	if(write)
	{
		//writing always in the save dir
		CString file = theFileControl->filecontrolsavedir + filename;
		if(!makeFile(file)) return false;
		return CFile::open(file, write);
	}
	else
	{
		//reading according to file control
		CString file = theFileControl->getLongFilename(filename);
		if(file == "") return false;
		return CFile::open(file, write);
	}
}

CString CDataFile::useExtern()
{
	CFile::close();
	return m_Filename;
}

CString getShortName(const CString &longname)
{
	int pos = longname.inStr(theFileControl->filecontroldatadir);
	if(pos >= 0)
		return longname.mid(pos + theFileControl->filecontroldatadir.length());

	pos = longname.inStr(theFileControl->filecontrolsavedir);
	if(pos >= 0)
		return longname.mid(pos + theFileControl->filecontrolsavedir.length());

	return "";
}

bool dataFileExists(const CString &filename, bool onlyLocal)
{
	if(onlyLocal)
	{
		return fileExists(theFileControl->filecontroldatadir + filename) ||
			fileExists(theFileControl->filecontrolsavedir + filename);
	}

	CDataFile f;
	bool ret = f.open(filename);
	f.close();
	return ret;
}

bool deleteDataFile(const CString &filename)
{
	CString fullname = theFileControl->filecontrolsavedir + filename;

	if(!fileExists(fullname)) return false;

	//Protection against evil removals (you never know)
	if(filename.inStr("..") >= 0) return false;

	printf("Deleting %s\n", fullname.c_str());
	if(unlink(fullname.c_str()) != 0) return false;

	return true;
}

bool copyDataFile(const CString &source, const CString &destination, bool srcIsData, bool dstIsData)
{
	CString src = source, dst = destination;

	if(srcIsData)
	{
		CDataFile f(src);
		src = f.useExtern();
	}

	if(dstIsData)
	{
		CDataFile f(dst, true); //write access
		dst = f.useExtern();
	}

	return copyFile(src, dst);
}

vector<CString> getDirContents(const CString &dir, const CString &ext)
{
	vector<CString> ret;

	CString fullname1 = theFileControl->filecontroldatadir + dir;
	CString fullname2 = theFileControl->filecontrolsavedir + dir;

	DIR *dir1 = opendir(fullname1.c_str());
	DIR *dir2 = opendir(fullname2.c_str());

	if(dir1 != NULL)
	{
		while(true)
		{
			struct dirent *entry = readdir(dir1);
			if(entry == NULL) break;

			CString entname = entry->d_name;
			//file extension check:
			if(ext == "" || (entname.inStr(ext) >= 0 && entname.inStr(ext) == (int)(entname.length() - ext.length()) ))
				ret.push_back(entname);
		}

		closedir(dir1);
	}

	if(dir2 != NULL)
	{
		while(true)
		{
			struct dirent *entry = readdir(dir2);
			if(entry == NULL) break;

			CString entname = entry->d_name;
			//file extension check:
			if(ext == "" || (entname.inStr(ext) >= 0 && entname.inStr(ext) == (int)(entname.length() - ext.length()) ))
			{
				//extra: make sure that we don't duplicate an existing entry
				bool duplicate = false;
				for(unsigned int i=0; i < ret.size(); i++)
					if(ret[i] == entname)
						{duplicate = true; break;}

				if(!duplicate)
					ret.push_back(entname);
			}
		}

		closedir(dir2);
	}

	return ret;
}
