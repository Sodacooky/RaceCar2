#include "RaceCar.h"

using namespace std;

int main(int argc, char* argv[])
{
	RaceCar app;
	return app.Main();
}

Car::Car()
{
	bIsPlayer = false;
	nTexId = 0;
	m_rect = { 0,0,0,0 };
}

void Car::Rect(const SDL_Rect& newRect)
{
	m_rect = newRect;
}

const SDL_Rect& Car::Rect()
{
	return m_rect;
}

void Car::RelativeMove(const SDL_Point& move)
{
	m_rect.x += move.x;
	m_rect.y += move.y;
}

int RaceCar::Main()
{
	__Init();

	while (true)
	{
		if (__StartPage() == 1) break;
		__Game();
	}

	return 0;
}

void RaceCar::__Init()
{
	GraphEngine::Init();
	GraphEngine::ChangeWinTitle(u8"RaceCar2");
	GraphEngine::ChangeWinRect({ SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600 });

	RandomEngine::Init();
}

int RaceCar::__StartPage()
{
	SDL_Point cursorSize;
	auto cursor = GraphEngine::LoadUnitFromUtf8Str(cursorSize, u8"¡ö", { 0,0,0 });
	cursor->Rect({ 360,360,40,40 });

	auto bg = GraphEngine::LoadUnitFromFile("Res/StartBG.png");
	bg->Rect({ 0,0,800,600 });

	SDL_Event msg;
	Controller ctrler(&msg);
	int nowSelection = 0;
	while (true)
	{
		SDL_PollEvent(&msg);
		ctrler.Handle();

		if (msg.type == SDL_QUIT) exit(0);

		if (ctrler.IsDowning(SDLK_DOWN))
		{
			if (nowSelection >= 1) nowSelection = 0;
			else nowSelection++;
		}
		else if (ctrler.IsDowning(SDLK_UP))
		{
			if (nowSelection <= 0) nowSelection = 1;
			else nowSelection--;
		}
		else if (ctrler.IsDowning(SDLK_RETURN))
		{
			break;
		}
		cursor->Rect({ 360,360 + nowSelection * 65,0,0 });

		GraphEngine::RenderClear();
		bg->RenderCopy();
		cursor->RenderCopy();
		GraphEngine::RenderPresent();
	}

	GraphEngine::FreeUnit(bg);
	GraphEngine::FreeUnit(cursor);

	return nowSelection;
}

int RaceCar::__Game()
{
	auto numspack = GraphEngine::LoadSqUnitPackFromFile("Res/Nums.png", true, { 255,255,255 });
	numspack->UnitSize(10);

	auto texpack = GraphEngine::LoadSqUnitPackFromFile("Res/texture.bmp",true,{255,255,255});
	texpack->UnitSize(16);

	SDL_Event msg;
	Controller ctrl(&msg);

	m_unScore = 0;
	m_ptCameraPos = { 0,-600 };

	Car player;
	player.bIsPlayer = true;
	player.nTexId = 20;
	player.Rect({ 360,-100,80,80 });
	m_listCar.clear();
	m_listCar.push_back(player);

	m_listMapBlock.clear();

	while (true)
	{
		//handle event
		SDL_PollEvent(&msg);
		if (msg.type == SDL_QUIT) exit(1);
		ctrl.Handle();
		if (ctrl.IsDowning(SDLK_ESCAPE)) break;
		//end handle event

		//control
		if (ctrl.IsHolding(SDLK_LEFT) || ctrl.IsDowning(SDLK_LEFT))  m_listCar.begin()->RelativeMove({ -8,0 });
		if (ctrl.IsHolding(SDLK_RIGHT) || ctrl.IsDowning(SDLK_RIGHT))  m_listCar.begin()->RelativeMove({ 8,0 });
		//end control

		//handle logic
		m_unScore++;
		m_ptCameraPos.y -= (m_unScore / 500) + 2;
		m_listCar.begin()->RelativeMove({ 0,(((int)m_unScore / 500) + 2) * -1 });
		__GnrtAndDelMapBlock(texpack);
		__GnrtAndDelCar();
		if (__IsHaveCollision()) break;
		//end handle logic

		//draw
		GraphEngine::RenderClear();
		__DrawMapBlocks();
		__DrawCar(texpack);
		__RenderScoreBoard(numspack);
		GraphEngine::RenderPresent();
		//end draw

		cout<<m_listCar.size()<<endl;
	}


	GraphEngine::FreeUnitPack(numspack);
	return 0;
}

void RaceCar::__RenderScoreBoard(GraphUnitPack* numspack)
{
	unsigned int score = m_unScore;
	int num;

	//draw background
	SDL_SetRenderDrawColor(GraphEngine::GetSDLRen(), 222, 195, 170, 255);
	SDL_RenderFillRect(GraphEngine::GetSDLRen(), &SDL_Rect({ 0,0,8 * 32,32 }));
	SDL_SetRenderDrawColor(GraphEngine::GetSDLRen(), 0, 0, 0, 255);
	//draw numbs
	for (int i = 7; i != -1; i--)
	{
		num = score % 10;
		score /= 10;
		numspack->Rect({ i * 32,0,32,32 });
		numspack->RenderCopy(num);
	}
}

void RaceCar::__GnrtAndDelMapBlock(GraphUnitPack* mapBlocksPack)
{
	const int MAX = 75;
	if (m_listMapBlock.size() < MAX * 2)
	{
		MapBlock mb;
		mb.pack = mapBlocksPack;
		int y = __GetTopBlockY() - BLOCKSIZE;
		for (int i = 0; i != 10; i++)
		{
			mb.pos = { i * BLOCKSIZE,y };
			mb.texID = RandomEngine::UniformRange(10, 19);
			m_listMapBlock.push_back(mb);
		}
	}

	if (m_listMapBlock.begin()->pos.y - m_ptCameraPos.y > 600)
	{
		for (int i = 0; i != 10; i++)
		{
			m_listMapBlock.erase(m_listMapBlock.begin());
		}
	}
}

int RaceCar::__GetTopBlockY()
{
	if (m_listMapBlock.size() > 0)
	{
		auto iter = m_listMapBlock.end();
		iter--;
		return iter->pos.y;
	}
	else return 0;
}

void RaceCar::__DrawMapBlocks()
{
	for (auto& block : m_listMapBlock)
	{
		block.RenderCopy(m_ptCameraPos);
	}
}

void RaceCar::__GnrtAndDelCar()
{
	const int MAX = 75;

	//Éú³É
	if (m_listCar.size()<MAX*2)
	{
		int percent = 10 + m_unScore / 1000;
		percent = percent > 50 ? 50 : percent;

		int carY = __GetTopCarY() - BLOCKSIZE;

		Car tmpCar;
		tmpCar.bIsPlayer = false;
		tmpCar.Rect({ 0,0,BLOCKSIZE,BLOCKSIZE });

		for (int x = 0; x != 10; x++)
		{
			if (RandomEngine::UniformRange(0, 99) < percent)
			{
				tmpCar.nTexId = RandomEngine::UniformRange(21, 29);
				tmpCar.Rect({ x * BLOCKSIZE,carY,BLOCKSIZE,BLOCKSIZE });
				m_listCar.push_back(tmpCar);
			}
		}
	}

	//ÒÆ¶¯
	for (auto& car : m_listCar)
	{
		if (!car.bIsPlayer)
		{
			car.RelativeMove({ 0,-1});
		}
	}

	//É¾³ý
	for (auto iter = m_listCar.begin(); iter != m_listCar.end();)
	{
		if (!iter->bIsPlayer)
		{
			if (iter->Rect().y - m_ptCameraPos.y > 600)
			{
				iter = m_listCar.erase(iter);
			}
			else iter++;
		}
		else iter++;
	}
}

int RaceCar::__GetTopCarY()
{
	if (m_listCar.size() <= 1) return -300;
	auto iter = m_listCar.end();
	iter--;
	return iter->Rect().y;
}

void RaceCar::__DrawCar(GraphUnitPack* pack)
{
	for (auto& car : m_listCar)
	{
		pack->Rect({car.Rect().x-m_ptCameraPos.x,car.Rect().y-m_ptCameraPos.y,0,0});
		pack->RenderCopy(car.nTexId);
	}
}

bool RaceCar::__IsHaveCollision(void)
{
	auto iterPlayer = m_listCar.begin();
	auto iter = m_listCar.begin();
	if(iterPlayer->Rect().x < 0 || iterPlayer->Rect().x>720) return true;
	while (iter != m_listCar.end())
	{
		if (!iter->bIsPlayer)
		{
			SDL_Point c1 = { iter->Rect().x,iter->Rect().y };
			SDL_Point c2 = { iterPlayer->Rect().x,iterPlayer->Rect().y };
			int rr = BLOCKSIZE-24;
			double d = sqrt(pow(c1.x - c2.x, 2) + pow(c1.y - c2.y, 2));
			if (d < rr) return true;
		}
		iter++;
	}
	return false;
}

bool MapBlock::IsCollisionWith(const SDL_Rect& rect)
{
	SDL_Point c1 = { pos.x + BLOCKSIZE / 2,pos.y + BLOCKSIZE / 2 };
	SDL_Point c2 = { rect.x + rect.w / 2,rect.y + rect.h / 2 };

	double rr = BLOCKSIZE / 2.0 + (rect.w + rect.h) / 4;
	double dis = sqrt(pow(c1.x - c2.x, 2) + pow(c1.y - c2.y, 2));

	if (dis <= rr)
	{
		return true;
	}
	return false;
}

void MapBlock::RenderCopy(const SDL_Point& camera)
{
	pack->Rect({ pos.x - camera.x,pos.y - camera.y,BLOCKSIZE,BLOCKSIZE });
	pack->RenderCopy(texID);
}
