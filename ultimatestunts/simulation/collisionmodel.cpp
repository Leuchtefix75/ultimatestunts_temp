/***************************************************************************
                          collisionmodel.cpp  -  A collision model
                             -------------------
    begin                : wo sep 24 2003
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

#include <stdio.h>
#include <math.h>
#include "collisionmodel.h"

CCollisionModel::CCollisionModel(){
}
CCollisionModel::~CCollisionModel(){
}

bool CCollisionModel::loadFromFile(CFile *f, CString subset, CMaterial **matarray)
{
	//Initial bounding volume state
	m_BSphere_r = 0.0;
	m_OBB_min = CVector(0,0,0);
	m_OBB_max = CVector(0,0,0);

	//Data for graphics etc.
	m_Filename = f->getName();
	m_Subset = subset;

	//Load faces from file
	f->reopen();
	ePrimitiveType currentType = None;
	while(true)
	{
		CString line = f->readl();
		if(line[0]=='\n') break;

		int sp = line.inStr(' ');
		if(sp>0)
		{
				CString lhs = line.mid(0, sp);
				CString rhs = line.mid(sp+1);

				if(lhs == "Lod")
				{
					if(rhs.inStr('c') < 0)
						while(true)
						{
							line = f->readl();
							if(line[0]=='\n') break;
							sp = line.inStr(' ');
							if(sp > 0 && line.mid(0, sp) == "Lod")
							{
								rhs = line.mid(sp+1);
								if(rhs.inStr('c') >= 0)
									break;
							}
						}
				}
				if(lhs=="Vertex" && currentType != None)
				{
					CVector v = rhs.toVector();

					CCollisionFace &theFace = m_Faces[m_Faces.size()-1];
					int index = theFace.size();
					switch(currentType)
					{
					case Triangles:
						if(index == 3)
						{
							m_Faces.push_back(CCollisionFace());
							m_Faces.back().reverse = false;
						}
						break;
					case Quads:
						if(index == 4)
						{
							m_Faces.push_back(CCollisionFace());
							m_Faces.back().reverse = false;
						}
						break;
					case Trianglestrip:
						if(index == 3)
						{
							m_Faces.push_back(CCollisionFace());
							CCollisionFace &newFace = m_Faces[m_Faces.size()-1];
							newFace.push_back(theFace[1]);
							newFace.push_back(theFace[2]);
							newFace.reverse = !theFace.reverse;
						}
						break;
					case Quadstrip:
						if(index == 4)
						{
							m_Faces.push_back(CCollisionFace());
							CCollisionFace &newFace = m_Faces[m_Faces.size()-1];
							CVector v2 = theFace[2];
							CVector v3 = theFace[3];
							theFace[2] = v3;
							theFace[3] = v2;
							newFace.push_back(v2);
							newFace.push_back(v3);
							newFace.reverse = false;
						}
						break;
					case Polygon:
					default:
						;
					}
					(m_Faces[m_Faces.size()-1]).push_back(v);
				}
				if(lhs=="Triangles")
					{currentType = Triangles; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
				if(lhs=="Quads")
					{currentType = Quads; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
				if(lhs=="Trianglestrip")
					{currentType = Trianglestrip; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
				if(lhs=="Quadstrip")
					{currentType = Quadstrip; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
				if(lhs=="Polygon")
					{currentType = Polygon; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
		}
		else
		{
			if(line=="Triangles")
				{currentType = Triangles; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
			if(line=="Quads")
				{currentType = Quads; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
			if(line=="Trianglestrip")
				{currentType = Trianglestrip; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
			if(line=="Quadstrip")
				{currentType = Quadstrip; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
			if(line=="Polygon")
				{currentType = Polygon; m_Faces.push_back(CCollisionFace()); m_Faces.back().reverse = false;}
		}
	}

	determineOBVs();
	determinePlaneEquations();

	return true;
}

void CCollisionModel::determineOBVs()
{
	//Generating oriented bounding volumes
	for(unsigned int i=0; i<m_Faces.size(); i++)
	{
		CCollisionFace &theFace = m_Faces[i];

		for(unsigned int j=0; j<theFace.size(); j++)
		{
			CVector &v = theFace[j];

			if(v.abs() > m_BSphere_r)
				m_BSphere_r = v.abs();

			if(v.x > m_OBB_max.x) m_OBB_max.x = v.x;
			if(v.y > m_OBB_max.y) m_OBB_max.y = v.y;
			if(v.z > m_OBB_max.z) m_OBB_max.z = v.z;
			if(v.x < m_OBB_min.x) m_OBB_min.x = v.x;
			if(v.y < m_OBB_min.y) m_OBB_min.y = v.y;
			if(v.z < m_OBB_min.z) m_OBB_min.z = v.z;
		}
	}
}

void CCollisionModel::determinePlaneEquations()
{
	//Determine a plane equation for each face
	for(unsigned int i=0; i<m_Faces.size(); i++)
	{
		CCollisionFace &theFace = m_Faces[i];

		//initial:
		theFace.d = 0.0;
		theFace.nor = CVector(0,0,0);

		if(theFace.size() < 3) break; //not a plane

		//the first point
		CVector p1 = theFace[0];

		//the second point: as far away from 1 as possible
		unsigned int index_2nd = 1;
		CVector p2 = theFace[index_2nd];
		float dist2 = (p2-p1).abs2();
		for(unsigned int j=2; j<theFace.size(); j++)
		{
			float d2 = (theFace[j] - p1).abs2();
			if(d2 > dist2)
				{p2 = theFace[j]; dist2 = d2; index_2nd = j;}
		}

		//the third point: try to maximise cross product
		CVector crosspr;
		float crosspr_abs2 = 0.0;
		bool reverse = false; //2 and 3 in reverse order
		CVector p1p2 = p2 - p1;
		for(unsigned int j=1; j<theFace.size(); j++)
		{
			const CVector &p3 = theFace[j];
			CVector cp = (p3-p1).crossProduct(p1p2);
			float abs2 = cp.abs2();

			if(abs2 > crosspr_abs2)
			{
				crosspr = cp;
				crosspr_abs2 = abs2;
				reverse = (j<index_2nd);
			}
		}

		if(crosspr_abs2 < 0.00001) break; //too small

		reverse ^= theFace.reverse;
		if(reverse) //2 and 3 in reverse order
			theFace.nor = crosspr.normal();
		else
			theFace.nor = -crosspr.normal();

		theFace.d = p1.dotProduct(theFace.nor);

		/*
		printf("  Face %d: p1=(%.2f,%.2f,%.2f) nor=(%.3f,%.3f,%.3f) d=%.2f\n",
			i,
			p1.x, p1.y, p1.z,
			theFace.nor.x, theFace.nor.y, theFace.nor.z,
			theFace.d
		);
		*/
	}
}