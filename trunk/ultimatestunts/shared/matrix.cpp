/***************************************************************************
                          matrix.cpp  -  A 3D rotation matrix class
                             -------------------
    begin                : Fri Jul 6 2001
    copyright            : (C) 2001 by CJP
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
#include "matrix.h"
//#include <stdio.h>
#include <math.h>

CMatrix::CMatrix()
{
    m_M = new float[16];

    reset();
}

CMatrix::CMatrix(CVector v)
{
	m_M = new float[16];

	//This function is NOT finished.
	//It currently only handles rotations around the y-axis.
	rotY(v.y);

	setElement(3,0,0.0);
	setElement(3,1,0.0);
	setElement(3,2,0.0);

	setElement(0,3,0.0);
	setElement(1,3,0.0);
	setElement(2,3,0.0);

	setElement(3,3,1.0);
}

CMatrix::CMatrix(CMatrix const &val)
{
	m_M = new float[16];
	operator=(val);
}

CMatrix::~CMatrix()
{
	delete [] m_M;
}

const float *CMatrix::gl_mtr() const
{return m_M;}

CMatrix const &CMatrix::operator=(CMatrix const &val)
{

	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++)
			setElement(i,j, val.Element(i,j) );

	return (*this);
}

CMatrix const &CMatrix::operator*=(CMatrix const &val)
{
  CMatrix temp = *this;

	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++)
			temp.setElement(i,j,
        Element(i,0) * val.Element(0,j) +
        Element(i,1) * val.Element(1,j) +
        Element(i,2) * val.Element(2,j)
      );

  *this = temp;

	return (*this);
}

float CMatrix::Element(int i, int j) const
{
	return *(m_M + i + 4*j);
}

void CMatrix::setElement(int i, int j, float e)
{
	*(m_M + i + 4*j) = e;
}

void  CMatrix::rotY ( float hoek)
{
	setElement(0,0,cos(hoek));	setElement(1,0,0.0);	setElement(2,0,-sin(hoek));
	setElement(0,1,0.0);				setElement(1,1,1.0);	setElement(2,1,0.0);
	setElement(0,2,sin(hoek));	setElement(1,2,0.0);	setElement(2,2,cos(hoek));
}

void  CMatrix::rotX ( float hoek)
{
	setElement(0,0,1.0);	setElement(0,1,0.0);				setElement(0,2,0.0);
	setElement(1,0,0.0);	setElement(1,1,cos(hoek));	setElement(1,2,-sin(hoek));
	setElement(2,0,0.0);	setElement(2,1,sin(hoek));	setElement(2,2,cos(hoek));
}

void CMatrix::reset()
{
    for (int i=0; i<4; i++)
    {
        for (int j=0; j<4; j++)
        {
            if (i==j)
                setElement(i,j,1.0);
            else
                setElement(i, j, 0.0);
        }
    }
}
