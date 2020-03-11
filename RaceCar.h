#pragma once

#include "Engine/Engine.h"
#include <list>
#include <map>
#include <sstream>
#include <math.h>

const int BLOCKSIZE = 80;

class Car
{
public:
	bool bIsPlayer;
	int nTexId;

	Car();

	void Rect(const SDL_Rect& newRect);
	const SDL_Rect& Rect();

	void RelativeMove(const SDL_Point& move);

private:
	SDL_Rect m_rect;
};

class MapBlock
{
public:
	GraphUnitPack* pack;
	SDL_Point pos;
	int texID;
	bool IsCollisionWith(const SDL_Rect& rect);
	void RenderCopy(const SDL_Point& camera);
};

class RaceCar
{
public:
	//RaceCar()=default;
	int Main();
private:
	std::list<Car> m_listCar;
	std::list<MapBlock> m_listMapBlock;
	unsigned int m_unScore;
	//摄像机跟随车子，所以车子的移动速度就是摄像机的移动速度
	SDL_Point m_ptCameraPos;
private:
	void __Init();

	int __StartPage();

	int __Game();
	//分数板绘制
	void __RenderScoreBoard(GraphUnitPack* numspack);
	//地图
	void __GnrtAndDelMapBlock(GraphUnitPack* mapBlocksPack);
	int __GetTopBlockY();
	void __DrawMapBlocks();
	//
	void __GnrtAndDelCar();
	int __GetTopCarY();
	void __DrawCar(GraphUnitPack* pack);
	//
	bool __IsHaveCollision(void);
};