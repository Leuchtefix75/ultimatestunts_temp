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
#include <math.h>
#include "collisiondata.h"
#include "world.h"
#include "usmacros.h"

CCollisionData::CCollisionData(const CWorld *w)
{
	m_World = w;
	m_FirstUpdate = true;
}

CCollisionData::~CCollisionData()
{
}

void CCollisionData::calculateCollisions()
{
	if(m_FirstUpdate)
	{
		m_FirstUpdate = false;
		calculateTrackBounds();
	}

	m_Events.clear();

	for(unsigned int i=0; i<m_World->m_MovObjs.size(); i++)
	{
		//Add an event array
		m_Events.push_back(CColEvents());
		m_Events[i].isHit = false;
	}

	//Collision with track bounds
	for(unsigned int i=0; i<m_World->m_MovObjs.size(); i++)
		ObjTrackBoundTest(i);

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

void CCollisionData::calculateTrackBounds()
{
	int minz = 0;
	int maxz = 0;

	int lth = m_World->m_L;
	int wth = m_World->m_W;
	int hth = m_World->m_H;
	for(int x = 0; x < lth; x++)
		for(int y = 0; y < wth; y++)
			for(int h = 0; h < hth; h++)
			{
				int z = m_World->m_Track[h + hth*(y + wth*x)].m_Z;
				if(z < minz) minz = z;
				if(z > maxz) maxz = z;
			}

	m_TrackMin = CVector(
		-0.5*TILESIZE,
		minz*VERTSIZE,
		-0.5*TILESIZE);
	m_TrackMax = CVector(
		((float)lth-0.5)*TILESIZE,
		(maxz+30)*VERTSIZE,
		((float)wth-0.5)*TILESIZE);
}

void CCollisionData::ObjTrackBoundTest(int n)
{
	CMovingObject *o = m_World->m_MovObjs[n];
	CVector pos = o->getPosition();
	CVector mom = o->getVelocity() * o->m_Mass;
	//TODO: per body collision
	//for(unsigned int i=0; i<o->m_Bodies.size(); i++)
	int i=0;
		{
			CVector p = pos + o->m_Bodies[i].m_Position;
			const CBound *b = m_World->m_MovObjBounds[o->m_Bodies[i].m_Body];
			float r = b->m_BSphere_r;

			if(p.x - r < m_TrackMin.x)
			{
				CCollision c;
				c.body = i;
				c.dp = CVector(-mom.x,0,0);
				c.dr = CVector(m_TrackMin.x-p.x+r,0,0);
				c.mat1 = c.mat2 = NULL;
				c.nor = CVector(-1,0,0);
				c.pos = CVector(-r,0,0);

				m_Events[n].isHit = true;
				m_Events[n].push_back(c);
			}
			if(p.y - r < m_TrackMin.y)
			{
				CCollision c;
				c.body = i;
				c.dp = CVector(0,-mom.y,0);
				c.dr = CVector(0,m_TrackMin.y-p.y+r,0);
				c.mat1 = c.mat2 = NULL;
				c.nor = CVector(0,-1,0);
				c.pos = CVector(0,-r,0);

				m_Events[n].isHit = true;
				m_Events[n].push_back(c);
			}
			if(p.z - r < m_TrackMin.z)
			{
				CCollision c;
				c.body = i;
				c.dp = CVector(0,0,-mom.z);
				c.dr = CVector(0,0,m_TrackMin.z-p.z+r);
				c.mat1 = c.mat2 = NULL;
				c.nor = CVector(0,0,-1);
				c.pos = CVector(0,0,-r);

				m_Events[n].isHit = true;
				m_Events[n].push_back(c);
			}
			if(p.x + r > m_TrackMax.x)
			{
				CCollision c;
				c.body = i;
				c.dp = CVector(-mom.x,0,0);
				c.dr = CVector(m_TrackMax.x-p.x-r,0,0);
				c.mat1 = c.mat2 = NULL;
				c.nor = CVector(1,0,0);
				c.pos = CVector(r,0,0);

				m_Events[n].isHit = true;
				m_Events[n].push_back(c);
			}
			if(p.y + r > m_TrackMax.y)
			{
				CCollision c;
				c.body = i;
				c.dp = CVector(0,-mom.y,0);
				c.dr = CVector(0,m_TrackMax.y-p.y-r,0);
				c.mat1 = c.mat2 = NULL;
				c.nor = CVector(0,1,0);
				c.pos = CVector(0,r,0);

				m_Events[n].isHit = true;
				m_Events[n].push_back(c);
			}
			if(p.z + r > m_TrackMax.z)
			{
				CCollision c;
				c.body = i;
				c.dp = CVector(0,0,-mom.z);
				c.dr = CVector(0,0,m_TrackMax.z-p.z-r);
				c.mat1 = c.mat2 = NULL;
				c.nor = CVector(0,0,1);
				c.pos = CVector(0,0,r);

				m_Events[n].isHit = true;
				m_Events[n].push_back(c);
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
	int trot = theTile.m_R;
	int thth = theTile.m_Z;
	CVector tilepos = CVector(xtile*TILESIZE, thth*VERTSIZE, ztile*TILESIZE);
	CTileModel *tilemodel = m_World->m_TileModels[tmodel];

	CMovingObject *obj = m_World->m_MovObjs[nobj];
	CVector pos = obj->getPosition();
	CVector prevpos = obj->getPreviousPosition();
	for(unsigned int i=0; i<obj->m_Bodies.size(); i++)
	{
		CVector p = pos + obj->m_Bodies[i].m_Position;
		CVector pp = prevpos + obj->m_Bodies[i].m_PreviousPosition;
		const CBound *b = m_World->m_MovObjBounds[obj->m_Bodies[i].m_Body];

		//Test bounding spheres
		if(!sphereTest(p, b, tilepos, tilemodel)) continue;

		//Per tile-face
		for(unsigned int tf=0; tf<tilemodel->m_Faces.size(); tf++)
		{
			//face-sphere collision test

			if(tilemodel->m_Faces[tf].nor.abs2() < 0.5) continue; //invalid plane
			CCollisionFace theFace = tilemodel->m_Faces[tf]; //a copy

			//rotate the copy
			for(unsigned int j=0; j<theFace.size(); j++)
				theFace[j] = tileRotate(theFace[j], trot);
			theFace.nor = tileRotate(theFace.nor, trot);

			CVector midpos = p - tilepos;
			float ddiff = midpos.dotProduct(theFace.nor) - theFace.d;
			CVector prevmidpos = pp - tilepos;
			float prevddiff = prevmidpos.dotProduct(theFace.nor) - theFace.d;
			float r = b->m_BSphere_r;

			if(ddiff > r)
				continue;
			if(ddiff < 0.0 && prevddiff < 0.0)
				continue;

			//check if it is inside the face
			CVector curcoll_pos = midpos - ddiff * theFace.nor;
			CVector prevcoll_pos = prevmidpos - prevddiff * theFace.nor;
			CVector coll_pos = ((prevddiff-r)*curcoll_pos + (r-ddiff)*prevcoll_pos) / (prevddiff-ddiff);
			float angle = 0.0;
			for(unsigned int j=1; j<theFace.size(); j++)
			{
				CVector p1 = theFace[j]-coll_pos;
				CVector p2 = theFace[j-1]-coll_pos;
				float inpr = p1.dotProduct(p2);
				inpr /= (p1.abs() * p2.abs());
				angle += acos(inpr);
			}
			{
				CVector p1 = theFace[0]-coll_pos;
				CVector p2 = theFace.back()-coll_pos;
				float inpr = p1.dotProduct(p2);
				inpr /= (p1.abs() * p2.abs());
				angle += acos(inpr);
			}
			if(angle < 6.0 || angle > 6.5) //!= 2*pi: outside face
				continue;

			CCollision c;

			c.nor = theFace.nor;
			c.pos = -ddiff * theFace.nor;
			c.mat1 = NULL;
			c.body = i;

			//position correction
			c.dr = c.nor * (r - ddiff);

			//determine momentum transfer
			{
				CVector v = obj->getVelocity().component(c.nor);
				float m = obj->m_Mass;
				c.dp = -responsefactor*m*v;
			}

			m_Events[nobj].isHit = true;
			m_Events[nobj].push_back(c);

		} //for tf
	} //for i

}

CVector CCollisionData::tileRotate(CVector v, int rot)
{
	switch(rot % 4)
	{
	case 0:
		return v;
	case 1:
		return CVector(v.z, v.y, -v.x);
	case 2:
		return CVector(-v.x, v.y, -v.z);
	case 3:
		return CVector(-v.z, v.y, v.x);
	}

	return v; //just to prevent warnings
}

bool CCollisionData::sphereTest(const CVector &p1, const CCollisionModel *b1, const CVector &p2, const CCollisionModel *b2)
{
	float abs2 = (p2-p1).abs2();
	float rsum = b1->m_BSphere_r + b2->m_BSphere_r;
	return abs2 < (rsum * rsum);
}