/***************************************************************************
                          collisiondata.cpp  -  data describing collision events
                             -------------------
    begin                : di sep 23 2003
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

#include "collisiondata.h"
#include "world.h"
#include "usmacros.h"

CCollisionData::CCollisionData(const CWorld *w)
{
	m_World = w;
}

CCollisionData::~CCollisionData()
{
}

void CCollisionData::calculateCollisions()
{
	m_Events.clear();

	for(unsigned int i=0; i<m_World->m_MovObjs.size(); i++)
	{
		//Add an event array
		m_Events.push_back(CColEvents());
		m_Events[i].isHit = false;
	}

	//Moving object to moving object collisions
	for(unsigned int i=0; i < m_World->m_MovObjs.size()-1; i++)
		for(unsigned int j=i+1; j < m_World->m_MovObjs.size(); j++)
			ObjObjTest(i,j);

	//Moving object to tile collisions
	int lengte = m_World->m_L;
	int breedte = m_World->m_W;
	int hoogte = m_World->m_H;
	for(unsigned int i=0; i < m_World->m_MovObjs.size(); i++)
	{
		CVector pos = m_World->m_MovObjs[i]->getPosition();
		int x = (int)(0.5 + (pos.x)/TILESIZE);
		int z = (int)(0.5 + (pos.z)/TILESIZE);

		int xmin = (x-1 < 0)? 0 : x-1;
		int xmax = (x+1 > lengte-1)? lengte-1 : x+1;
		int zmin = (z-1 < 0)? 0 : z-1;
		int zmax = (z+1 > breedte-1)? breedte-1 : z+1;

		for(x=xmin; x<=xmax; x++)
			for(z=zmin; z<=zmax; z++)
				for(int h=0; h<hoogte; h++)
				{
					ObjTileTest(i, x, z, h);
				}
	}
}

#define elasticity 0.6
#define responsefactor (1.0 + elasticity)

void CCollisionData::ObjObjTest(int n1, int n2)
{
	CMovingObject *o1 = m_World->m_MovObjs[n1];
	CMovingObject *o2 = m_World->m_MovObjs[n2];
	CVector pos1 = o1->getPosition();
	CVector pos2 = o2->getPosition();

	//TODO: per body collision
	//for(unsigned int i=0; i<o1->m_Bodies.size(); i++)
	//	for(unsigned int j=0; j<o2->m_Bodies.size(); j++)
	int i=0, j=0;
		{
			CVector p1 = pos1 + o1->m_Bodies[i].m_Position;
			CVector p2 = pos2 + o2->m_Bodies[j].m_Position;
			const CBound *b1 = m_World->m_MovObjBounds[o1->m_Bodies[i].m_Body];
			const CBound *b2 = m_World->m_MovObjBounds[o2->m_Bodies[i].m_Body];
			if(sphereTest(p1, b1, p2, b2))
			{
				printf("Collision between %d and %d\n", n1, n2);
				CCollision c1, c2;

				c1.nor = p2 - p1;
				c1.nor.normalise();
				c2.nor = -c1.nor;

				c1.pos = b1->m_BSphere_r * c1.nor;
				c2.pos = b2->m_BSphere_r * c2.nor;

				c1.mat1 = c1.mat2 = NULL;
				c2.mat1 = c1.mat1;
				c2.mat2 = c1.mat2;

				c1.body = i;
				c2.body = j;

				//position correction
				float dr = 0.5 * (b1->m_BSphere_r + b2->m_BSphere_r - (p2-p1).abs());
				c1.dr = c1.nor * -dr;
				c2.dr = c2.nor * -dr;

				//determine momentum transfer
				{
					CVector v1 = o1->getVelocity().component(c1.nor);
					CVector v2 = o2->getVelocity().component(c1.nor);
					float m1 = o1->m_Mass;
					float m2 = o2->m_Mass;
					CVector vcg = (m1*v1 + m2*v2)/(m1 + m2);
					c1.dp = responsefactor*m1*(vcg - v1);

					c2.dp = -c1.dp;
				}

				m_Events[n1].isHit = true;
				m_Events[n1].push_back(c1);
				m_Events[n2].isHit = true;
				m_Events[n2].push_back(c2);
			}
		}
}

void CCollisionData::ObjTileTest(int nobj, int xtile, int ztile, int htile)
{
	int tindex = htile + m_World->m_H*(ztile + m_World->m_W*xtile);
	const CTile &theTile = m_World->m_Track[tindex];
	int tmodel = theTile.m_Model;
	//int trot = theTile.m_R;
	int thth = theTile.m_Z;
	CVector tilepos = CVector(xtile*TILESIZE, thth*VERTSIZE, ztile*TILESIZE);
	CTileModel *tilemodel = m_World->m_TileModels[tmodel];

	CMovingObject *obj = m_World->m_MovObjs[nobj];
	CVector pos = obj->getPosition();
	//TODO: per body collision
	//for(unsigned int i=0; i<obj->m_Bodies.size(); i++)
	int i=0;
	{
		CVector p = pos + obj->m_Bodies[i].m_Position;
		const CBound *b = m_World->m_MovObjBounds[obj->m_Bodies[i].m_Body];
		if(sphereTest(p, b, tilepos, tilemodel))
		{
			printf("Collision between %d and tile\n", nobj);
			CCollision c;

			c.nor = tilepos - p;
			c.nor.normalise();

			c.pos = b->m_BSphere_r * c.nor;

			c.mat1 = NULL;

			c.body = i;

			//position correction
			float dr = b->m_BSphere_r + tilemodel->m_BSphere_r - (tilepos-p).abs();
			c.dr = c.nor * -dr;

			//determine momentum transfer
			{
				CVector v = obj->getVelocity().component(c.nor);
				float m = obj->m_Mass;
				c.dp = -responsefactor*m*v;
			}

			m_Events[nobj].isHit = true;
			m_Events[nobj].push_back(c);
		}
	}

}

bool CCollisionData::sphereTest(const CVector &p1, const CCollisionModel *b1, const CVector &p2, const CCollisionModel *b2)
{
	float abs2 = (p2-p1).abs2();
	float rsum = b1->m_BSphere_r + b2->m_BSphere_r;
	return abs2 < (rsum * rsum);
}
