#include <stdio.h>
#include <stdint.h>
#include "glm/glm.hpp"

namespace spiral
{

char tab[40*25];
uint16_t startpoints[1000];
int startpointcount = 0;

int dronestartcount = 0;

using glm::ivec2;
using glm::ivec3;


ivec2 dronepoints[1000];


class drone_t
{
public:
	ivec3 drone;
	bool enabled = false;
	int life;
	int rookieness = 0;

	void iterate()
	{
		int i;
		ivec3 droneo = drone;

		int triedz[4] = {0,0,0,0};

		int deadend = 0;

		while(1)
		{
			drone.x = droneo.x;
			drone.y = droneo.y;

//			drone.z += rand() % 1;
			drone.z %= 4;

			deadend = 0;
			for (i = 0; i < 4; i++)
				deadend += triedz[i];

			if (deadend == 4)
				break;

			triedz[drone.z] = 1;

//			printf("consideration drone: %i,%i,%i\n", drone.x, drone.y, drone.z);

			switch(drone.z)
			{
				case 0: drone.y--; break;
				case 1: drone.x++; break;
				case 2: drone.y++; break;
				case 3: drone.x--; break;
			}

			if (drone.x < 0)
			{
				drone.z++;
				continue;
			}
			if (drone.x > 39)
			{
				drone.z++;
				continue;
			}
			if (drone.y < 0)
			{
				drone.z++;
				continue;
			}
			if (drone.y > 24)
			{
				drone.z++;
				continue;
			}

			if (tab[drone.y * 40 + drone.x] != ' ')
			{
				drone.z++;
				continue;
			}

			break;
		}

		if (deadend == 4)
		{
			tab[droneo.y * 40 + droneo.x] = 'X';
			initnew(0);
		}
		else
		{
			char dirtab[] = {'^', '>', 'v', '<'};
			tab[drone.y * 40 + drone.x] = 'x';
			tab[droneo.y * 40 + droneo.x] = dirtab[drone.z];

			life++;

			if (life > 10000)
			{
//				drone.z = rand() % 4;
				drone.z++;
				life = 0;
			}
		}

//		printf("drone: %i,%i,%i (%i)\n", drone.x, drone.y, drone.z, enabled);

	}

	void initnew(int id)
	{
		ivec2 freepos[40*25];
		int freecount = 0;
		int i;
		for (i = 0; i < 40*25; i++)
		{
			if (tab[i] == ' ')
			{
				freepos[freecount] = ivec2(i % 40, i / 40);
				freecount++;
			}
		}
		if (freecount == 0)
		{
			enabled = false;
			return;
		}


		int nyt = rand() % freecount;
//		nyt = freecount - 1;

//		int nyt = (freecount / 2) % freecount;
		drone.x = freepos[nyt].x;
		drone.y = freepos[nyt].y;
		drone.z = rand() % 4;
//		drone.z = 1;
		enabled = true;
		life = 0;


		rookieness++;


		tab[drone.y * 40 + drone.x] = 'S';
		dronepoints[dronestartcount] = ivec2(drone.x, drone.y);
		dronestartcount++;
	}
};

int make(void)
{

	int i;
	int x,y;
	for(i = 0; i < 40*25; i++)
		tab[i] = ' ';

	const int dronecount = 8;

	srand(1200);

	drone_t drones[dronecount];
	for (i = 0; i < dronecount; i++)
		drones[i].initnew(i * 5 + 3);


	while(1)
	{
//		printf("\033[2J\033[;H");


		int alive = 0;
		for (i = 0; i < dronecount; i++)
		{
			drones[i].iterate();
			if (drones[i].enabled)
				alive++;
		}
/*
		for (y = 0; y < 25; y++)
		{
			for (x = 0; x < 40; x++)
			{
				printf("%c", tab[y * 40 + x]);
			}
			printf("\n");
		}
		fflush(stdout);
		printf("dronecount: %i\n", dronestartcount);
*/

		if (alive == 0)
			break;;

//		usleep(100000);
	}
		printf("dronecount: %i\n", dronestartcount);

	startpointcount = 0;
	int cycle;

	for (cycle = 0; cycle < 2; cycle++)
	{
		for (i = 0; i < dronestartcount; i++)
		{
			ivec2 p = dronepoints[i];
			//printf("%i: %i, %i\n", i, p.x, p.y);
			startpoints[startpointcount] = p.y * 40 + p.x;
			startpointcount++;
			if (startpointcount >= 256)
				goto nytriittaa;
		}
	}
	nytriittaa:;

		for (i = 0; i < startpointcount; i++)
		{
			int sec = (startpointcount + i - rand() % 20) % startpointcount;

			uint16_t p = startpoints[sec];
			startpoints[sec] = startpoints[i];
			startpoints[i] = p;
		}


	return 0;
}
}