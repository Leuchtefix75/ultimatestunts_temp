/***************************************************************************
                          gui.h  -  The graphical user interface: menu's etc.
                             -------------------
    begin                : vr jan 31 2003
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

#ifndef GUI_H
#define GUI_H

#include <vector>
namespace std {}
using namespace std;

#include "lconfig.h"
#include "widget.h"
#include "winsystem.h"

/**
  *@author CJP
  */

class CGUI : public CWidget
{
public:
	CGUI(const CLConfig &conf, CWinSystem *winsys);
	virtual ~CGUI();

	virtual int onMouseMove(int x, int y);
	virtual int onMouseClick(int x, int y, unsigned int buttons);
	virtual int onKeyPress(int key);
	virtual int onResize(int w, int h);
	virtual int onRedraw();

	virtual void start(); //returns when an exit command is given

protected:
	CWinSystem *m_WinSys;
	CWidget *m_ChildWidget;

	void enter2DMode();
	void leave2DMode();
	
private:
	bool m_in2DMode;
};

#endif