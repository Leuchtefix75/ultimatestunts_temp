/***************************************************************************
                          main.cpp  -  Stunts-Server main
                             -------------------
    begin                : wo nov 20 23:23:28 CET 2002
    copyright            : (C) 2002 by CJP
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "simulation.h"
#include "serversim.h"
#include "clientsim.h"
#include "physics.h"

#include "world.h"

#include "objectchoice.h"
#include "aiplayercar.h"

#include "lconfig.h"

CWorld *world;
vector<CPlayer *> players;
CSimulation *sim, *subsim1;

CString getInput(CString question="")
{
	printf("%s", question.c_str());
	char input[80];
	scanf("%s", input);
	return input;
}

bool addPlayer()
{
	CPlayer *p = new CAIPlayerCar(world);

	CObjectChoice choice;

	int id = sim->addPlayer(choice);
	p->m_MovingObjectId = id;
	p->m_PlayerId = 0;
	if(id < 0)
	{
		printf("Sim doesn't accept player\n");
		delete p;
		return false;
	}

	players.push_back(p);
	return true;
}

bool mainloop()
{
	/*
 //Debugging 'display'
	printf("\033[1H");
	printf("\033[1G");
	printf("**********\n");
	for(unsigned int i=0; i<world->m_MovObjs.size(); i++)
	{
		CMovingObject *mo = world->m_MovObjs[i];
		CVector r = mo->getPosition();
		CVector v = mo->getVelocity();
		printf("Object %d: position (%.2f,%.2f,%.2f), velocity (%.2f,%.2f,%.2f)\n",
		               i,            r.x, r.y, r.z,       v.x, v.y, v.z);
	}
    	printf("**********\n");
	*/

	for(unsigned int i=0; i<players.size(); i++)
		players[i]->update(); //Makes moving decisions

	sim->update(); //Modifies world object

	return false; //One loop
}

int main(int argc, char *argv[])
{
	printf("Welcome to the " PACKAGE " server version " VERSION "\n");

	int port = 1500;
	CString trackfile;
	unsigned int remote_players;
	unsigned int ai_players;

	printf("Loading configuration file\n");
	CLConfig conffile(argc, argv);
	conffile.setFilename("ultimatestunts.conf");

	printf("Parsing input arguments\n");
	for(int i=0; i<argc; i++)
	{
		CString arg = argv[i];
		int is = arg.inStr('=');
		if(is > 0)
		{
			CString lhs = arg.mid(0,is);
			CString rhs = arg.mid(is+1);
			if(lhs == "port")
				port = rhs.toInt();
		}
	}

	printf("Creating world object\n");
	world = new CWorld(conffile);

	CString answ = getInput("Do you want to use a \"super-server\" (y/n)? ");
	printf("\nYou entered %c\n", answ[0]);
	if(answ[0]=='y' || answ[0]=='Y')
	{
		CString h = getInput("Enter the super-server's hostname: ");
		int p = getInput("Enter the super-server's port number: ").toInt();

		printf("Creating client-type simulation\n");
		subsim1 = new CClientSim(world, h, p);
		trackfile = ((CClientSim *)subsim1)->getTrackname();
	}
	else
	{
		printf("Creating physics simulation\n");
		subsim1 = new CPhysics(world);

		trackfile = getInput("Please enter the track file: ");
		printf("\nYou entered %s\n", trackfile.c_str());
	}

	remote_players = getInput("Enter the number of remote players: ").toInt();
	printf("\nYou entered %d\n", remote_players);
	ai_players = getInput("Enter the number of AI players: ").toInt();
	printf("\nYou entered %d\n", ai_players);

	printf("\nCreating server simulation\n");
	{
		CServerSim *ssim = new CServerSim(world, port);
		ssim->addSubSim(subsim1);
		sim = (CSimulation *)ssim;
	}

	printf("\nLoading track data\n");
	world->loadTrack(trackfile);

	for(unsigned int i=0; i<ai_players; i++)
		addPlayer();

	if(!sim->loadObjects())
		printf("Error while loading moving objects\n");

	printf("\nEntering mainloop\n");

	//Creating some white space
	for(int i=1; i<40; i++)
		printf("\n");

	while(mainloop());

	printf("\nLeaving mainloop\n");

	printf("\nDeleting simulation\n");
	delete sim;

	printf("\nDeleting players\n");
	for(unsigned int i=0; i<players.size(); i++)
		delete players[i];
	players.clear();

	printf("\nDeleting world\n");
	delete world;

  return EXIT_SUCCESS;
}