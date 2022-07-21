#pragma once

//#include "mybsp.h"
#include "libtcod.hpp"
#include <curses.h>

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;

enum CONTROLS 
{
	UP = 'w', DOWN = 's', LEFT = 'a', RIGHT = 'd', QUIT = 'q'
};

struct Tile 
{
	bool canWalk; // can we walk through this tile?
	Tile() : canWalk(false) {}
};

class Map 
{
public:
	int height, width;

	Map(int height, int width);
	~Map();

	bool isWall(int y, int x) const;
	void render() const;
protected:
	Tile* tiles;

	friend class BspListener;

	void dig(int y1,int x1, int y2,int x2);
	void createRoom(bool first, int y1, int x1, int y2, int x2);
	void setWall(int y, int x);
};

//class BspListener_2 : public BspCallback
//{
//private:
//	Map& map; // a map to dig
//	int roomNum; // room number
//	int lasty = 0, lastx = 0; // center of the last room
//
//public:
//	BspListener(Map& map) : map(map), roomNum(0) {}
//	/*bool visitNode();*/
//	
//	bool visitNode(Bsp* node, void* userData)
//	{
//		if (node->isLeaf())
//		{
//			int y, x, h, w;
//			// dig a room
//
//			TCODRandom* rng = TCODRandom::getInstance();
//
//			h = rng->getInt(6, node->h - 2);
//			w = rng->getInt(6, node->w - 2);
//			y = rng->getInt(node->y + 1, node->y + node->h - h - 1);
//			x = rng->getInt(node->x + 1, node->x + node->w - w - 1);
//
//			map.createRoom(roomNum == 0, y, x, y + h - 1, x + w - 1);
//
//			if (roomNum != 0)
//			{
//				// dig a corridor from last room
//				
//				map.dig(lasty, x + w / 2, y + h / 2, x + w / 2);
//				map.dig(lasty, lastx, lasty, x + w / 2);
//			}
//			lastx = x + w / 2;
//			lasty = y + h / 2;
//			roomNum++;
//		}
//		return true;
//	}
//};

class BspListener : public ITCODBspCallback 
{
private:
	Map& map; // a map to dig
	int roomNum; // room number
	int lastx, lasty; // center of the last room
public:
	BspListener(Map& map) : map(map), roomNum(0) {}

	bool visitNode(TCODBsp* node, void* userData)
	{

		if (node->isLeaf())
		{
			//printw("isLeaf\n");
			//printw("%u",node->isLeaf());
			
			int x, y, w, h;
			// dig a room
			TCODRandom* rng = TCODRandom::getInstance();
			w = rng->getInt(ROOM_MIN_SIZE, node->w - 2);//random int from min size to width - 2
			h = rng->getInt(ROOM_MIN_SIZE, node->h - 2);//random int from min size to height - 2
			x = rng->getInt(node->x + 1, node->x + node->w - w - 1);//from node x + 1 to node x + node width - width - 1
			y = rng->getInt(node->y + 1, node->y + node->h - h - 1);//from node y + 1 to node x + node height - width - 1

			//DEBUG rng
			printw("w = %u||",w);
			printw("h = %u||",h);
			printw("x = %u||",x);
			printw("y = %u||",y);
			printw("roomnum = %u\n", roomNum);

			map.createRoom
			(
				roomNum == 0,
				x,
				y,
				x + w - 1,
				y + h - 1
			);



			//DEBUG createRoom


			if (roomNum != 0) 
			{
				// dig a corridor from last room

				map.dig(
					lasty,
					lastx,
					lasty,
					x + w / 2
				);
				map.dig(
					lasty,
					x + w / 2,
					y + h / 2,
					x + w / 2
				);
			}
			lastx = x + w / 2;
			lasty = y + h / 2;
			roomNum++;
		}
		return true;
	}
};

class Actor
{
public:
	int y, x;//position on map
	int ch;//ascii code
	int col; //color

	Actor(int y ,int x, int ch, int col);
	void render() const;
};

class Engine
{
public:
	/*Engine* engine;*/
	/*bool quit = false;*/
	std::vector<Actor*> actors;
	Actor* player;
	Map* map;

	Engine();
	~Engine();
	void update();
	void render();
};

Engine engine;
//extern Engine engine;