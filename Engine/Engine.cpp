#include "Engine.h"

/////////////GraphEngine////////////////
SDL_Window* GraphEngine::sm_pWin = nullptr;
SDL_Renderer* GraphEngine::sm_pRen = nullptr;
TTF_Font* GraphEngine::sm_pFont;
SDL_Rect GraphEngine::sm_rtWinRect;
///////////////RandomEngine////////////////
std::default_random_engine RandomEngine::sm_dre;
/////////////AudioEngine///////////////////
Mix_Music* AudioEngine::sm_pMusBGM;
int AudioEngine::sm_nBGMVolume;
bool AudioEngine::sm_bChannelUsed[64];

bool GraphEngine::Init(void)
{
	//normal sdl init
	//in not secure way

	//init sys
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		return false;
	}

	//init win
	sm_pWin = SDL_CreateWindow(u8"EngineWindows",
							   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							   800, 480,
							   SDL_WINDOW_SHOWN);
	if (sm_pWin == nullptr)
	{
		return false;
	}

	//init renderer
	sm_pRen = SDL_CreateRenderer(sm_pWin, -1,
								 SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (sm_pRen == nullptr)
	{
		return false;
	}

	//init font
	if (TTF_Init() != 0)
	{
		return false;
	}
	sm_pFont = TTF_OpenFont("C:/Windows/fonts/arial.ttf", 32);
	if (sm_pFont == nullptr)
	{
		//无法打开则提供第二选择，在exe文件夹下的font.ttf文件
		std::cout << "无法打开C:/Windows/fonts/arial.ttf，正在尝试打开程序目录下的font.ttf..." << std::endl;
		sm_pFont = TTF_OpenFont("font.ttf", 32);
		if (sm_pFont == nullptr)
		{
			std::cout << "无法打开程序目录下的font.ttf..." << std::endl;
			return false;
		}
	}
	//done
	return true;
}

void GraphEngine::Quit()
{
	SDL_DestroyRenderer(sm_pRen);
	SDL_DestroyWindow(sm_pWin);
	SDL_Quit();
	RenderClear();
}

void GraphEngine::ChangeWinRect(const SDL_Rect& rtWin)
{
	sm_rtWinRect = rtWin;
	SDL_SetWindowSize(sm_pWin, sm_rtWinRect.w, sm_rtWinRect.h);
	SDL_SetWindowPosition(sm_pWin, sm_rtWinRect.x, sm_rtWinRect.y);
}

void GraphEngine::ChangeWinTitle(const char* title)
{
	SDL_SetWindowTitle(sm_pWin, title);
}

void GraphEngine::RenderPresent(void)
{
	//present
	SDL_RenderPresent(sm_pRen);
}

void GraphEngine::RenderClear(void)
{
	//clear
	SDL_RenderClear(sm_pRen);
}

GraphUnit* GraphEngine::LoadUnitFromFile(const char* filename, bool use_key, const SDL_Color& key)
{
	SDL_Surface* pSur = IMG_Load(filename);
	if (pSur == nullptr)
	{
		throw std::string("LoadUnitFromFile() couldn't load file:\n" + std::string(IMG_GetError()));
		return nullptr;
	}

	SDL_SetColorKey(pSur, use_key, SDL_MapRGB(pSur->format, key.r, key.g, key.b));
	SDL_Texture* pTex = SDL_CreateTextureFromSurface(sm_pRen, pSur);
	if (pTex == nullptr)
	{
		SDL_FreeSurface(pSur);
		throw std::string("LoadUnitFromFile() failed to create sdl_texture:\n" + std::string(SDL_GetError()));
		return nullptr;
	}

	SDL_FreeSurface(pSur);

	auto pgu = new(std::nothrow) GraphUnit(sm_pRen, pTex);//nullptr not worried
	if (pgu == nullptr)
	{
		SDL_DestroyTexture(pTex);
		throw std::string("LoadUnitFromMem() couldb't allocate mem for graphunit");
		return nullptr;
	}

	return pgu;
}

GraphUnit* GraphEngine::LoadUnitFromMem(void* mem, unsigned int mem_size, bool use_key, const SDL_Color& key)
{
	if (mem == nullptr)
	{
		throw std::string("LoadUnitFromMem() null mem pointer");
		return nullptr;
	}

	SDL_RWops* rw = SDL_RWFromMem(mem, mem_size);
	if (rw == nullptr)
	{
		throw std::string("LoadUnitFromMem() failed read mem");
		return nullptr;
	}

	SDL_Surface* pSur = IMG_Load_RW(rw, true);//stream closed?
	if (pSur == nullptr)
	{
		throw std::string("LoadUnitFromMem() failed to create sdL_surface: " + std::string(IMG_GetError()));
		return nullptr;
	}

	SDL_SetColorKey(pSur, use_key, SDL_MapRGB(pSur->format, key.r, key.g, key.b));
	SDL_Texture* pTex = SDL_CreateTextureFromSurface(sm_pRen, pSur);
	if (pTex == nullptr)
	{
		SDL_FreeSurface(pSur);
		SDL_RWclose(rw);
		throw std::string("LoadUnitFromMem() failed to create sdL_surface: " + std::string(SDL_GetError()));
		return nullptr;
	}

	SDL_FreeSurface(pSur);
	SDL_RWclose(rw);

	auto pgu = new(std::nothrow) GraphUnit(sm_pRen, pTex);
	if (pgu == nullptr)
	{
		SDL_DestroyTexture(pTex);
		throw std::string("LoadUnitFromMem() couldn't allocate mem for graphunit");
		return nullptr;
	}

	return pgu;
}

GraphUnit* GraphEngine::LoadUnitFromAnsiStr(SDL_Point& size, const char* ansi_cstr,
											const SDL_Color& cl)
{
	int wcstr_size = MultiByteToWideChar(CP_ACP, 0, ansi_cstr, -1, nullptr, 0);
	WCHAR* wcstr = new(std::nothrow) WCHAR[wcstr_size];
	if (wcstr == nullptr)
	{
		std::cout << "StringToUnit() couldn't allocate mem for wcstr" << std::endl;
		return nullptr;
	}
	MultiByteToWideChar(CP_ACP, 0, ansi_cstr, -1, wcstr, wcstr_size);

	auto sur = TTF_RenderUNICODE_Solid(sm_pFont, (Uint16*)wcstr, cl);
	auto tex = SDL_CreateTextureFromSurface(sm_pRen, sur);
	SDL_FreeSurface(sur);

	TTF_SizeUNICODE(sm_pFont, (Uint16*)wcstr, &size.x, &size.y);
	size.x /= 2;
	size.y /= 2;

	delete[] wcstr;

	auto gu = new(std::nothrow) GraphUnit(sm_pRen, tex);
	if (gu == nullptr)
	{
		std::cout << "LoadUnitFromAnsiStr() couldn't allocate mem for graphunit" << std::endl;
		SDL_DestroyTexture(tex);
		return nullptr;
	}

	return gu;
}

GraphUnit* GraphEngine::LoadUnitFromUtf8Str(SDL_Point& size, const char* utf8_cstr,
											const SDL_Color& cl)
{
	TTF_SizeUTF8(sm_pFont, utf8_cstr, &size.x, &size.y);
	size.x /= 2;
	size.y /= 2;

	auto sur = TTF_RenderUTF8_Solid(sm_pFont, utf8_cstr, cl);
	auto tex = SDL_CreateTextureFromSurface(sm_pRen, sur);
	SDL_FreeSurface(sur);

	auto gu = new(std::nothrow) GraphUnit(sm_pRen, tex);
	if (gu == nullptr)
	{
		std::cout << "LoadUnitFromUtf8Str() couldn't allocate mem for graphunit" << std::endl;
		SDL_DestroyTexture(tex);
		return nullptr;
	}

	return gu;
}

GraphUnitPack* GraphEngine::LoadSqUnitPackFromFile(const char* filename, bool use_key, const SDL_Color& key)
{
	SDL_Surface* sur = IMG_Load(filename);
	if (sur == nullptr)
	{
		throw std::string("LoadSqUnitPackFromFile() couldn't load file:\n" + std::string(IMG_GetError()));
		return nullptr;
	}
	SDL_SetColorKey(sur, use_key, SDL_MapRGB(sur->format, key.r, key.g, key.b));

	SDL_Texture* tex = SDL_CreateTextureFromSurface(sm_pRen, sur);
	if (tex == nullptr)
	{
		SDL_FreeSurface(sur);
		throw std::string("LoadSqUnitPackFromFile() failed to create surface\n" + std::string(SDL_GetError()));
		return nullptr;
	}

	auto pPack = new(std::nothrow) GraphUnitPack(sm_pRen, tex, { sur->w,sur->h });
	if (pPack == nullptr)
	{
		SDL_FreeSurface(sur);
		SDL_DestroyTexture(tex);
		throw std::string("LoadSqUnitPackFromFile() failed to allocate mem for graphunitpack");
		return nullptr;
	}
	SDL_FreeSurface(sur);
	return pPack;
}

GraphUnitPack* GraphEngine::LoadSqUnitPackFromMem(void* mem, unsigned int mem_size,
												  bool use_key, const SDL_Color& key)
{
	if (mem == nullptr)
	{
		throw std::string("LoadSqUnitPackFromMem() null mem pointer");
		return nullptr;
	}

	SDL_RWops* rw = SDL_RWFromMem(mem, mem_size);
	if (rw == nullptr)
	{
		throw std::string("LoadSqUnitPackFromMem() failed to read mem");
		return nullptr;
	}

	SDL_Surface* sur = IMG_Load_RW(rw, true);//stream closed?
	if (sur == nullptr)
	{
		throw std::string("LoadSqUnitPackFromFile() couldn't load file:\n" + std::string(IMG_GetError()));
		return nullptr;
	}
	SDL_SetColorKey(sur, use_key, SDL_MapRGB(sur->format, key.r, key.g, key.b));

	SDL_Texture* tex = SDL_CreateTextureFromSurface(sm_pRen, sur);
	if (tex == nullptr)
	{
		throw std::string("LoadSqUnitPackFromFile() failed to create surface:\n" + std::string(SDL_GetError()));
		return nullptr;
	}

	auto pPack = new(std::nothrow) GraphUnitPack(sm_pRen, tex, { sur->w,sur->h });
	if (pPack == nullptr)
	{
		SDL_FreeSurface(sur);
		SDL_DestroyTexture(tex);
		throw std::string("LoadSqUnitPackFromFile() failed to allocate mem for graphunitpack");
		return nullptr;
	}
	SDL_FreeSurface(sur);
	return pPack;
}

void GraphEngine::FreeUnit(GraphUnit* gu)
{
	delete gu;
	gu = nullptr;
}

void GraphEngine::FreeUnitPack(GraphUnitPack* gup)
{
	delete gup;
	gup = nullptr;
}

SDL_Window* GraphEngine::GetSDLWin()
{
	return sm_pWin;
}

SDL_Renderer* GraphEngine::GetSDLRen()
{
	return sm_pRen;
}

void RandomEngine::Init()
{
	sm_dre.seed((unsigned int)&sm_dre);
}

int RandomEngine::UniformRange(int min, int max)
{
	std::uniform_int_distribution<int> d(min, max);
	return d(sm_dre);
}

int RandomEngine::NormalRange(int mu, int sigma)
{
	std::normal_distribution<>d(mu, sigma);
	return static_cast<int>(d(sm_dre));
}

bool RandomEngine::Half()
{
	std::uniform_int_distribution<int> d(0, 1);
	if (d(sm_dre) == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void RandomEngine::SetNewSeed()
{
	sm_dre.seed(SDL_GetTicks());
}

void RandomEngine::SetNewSeed(unsigned int seed)
{
	sm_dre.seed(seed);
}

bool AudioEngine::Init()
{
	if (Mix_Init(MIX_INIT_OGG | MIX_INIT_MID) == 0)
	{
		return false;
	}
	//不知道下面的错误是什么
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);
	Mix_AllocateChannels(8);
	return false;
}

void AudioEngine::Quit()
{
	Mix_AllocateChannels(0);
	Mix_FreeMusic(sm_pMusBGM);
	Mix_CloseAudio();
	Mix_Quit();
}

void AudioEngine::PlayBGM(int fade_ms)
{
	Mix_PlayMusic(sm_pMusBGM, 0);
}

void AudioEngine::RewindBGM()
{
	Mix_RewindMusic();
}

void AudioEngine::StopBGM(int fade_ms)
{
	Mix_FadeOutMusic(fade_ms);
}

void AudioEngine::PauseBGM(void)
{
	Mix_PauseMusic();
}

void AudioEngine::ResumeBGM()
{
	Mix_ResumeMusic();
}

void AudioEngine::LoadBGMFromFile(const char* filename)
{
	//del old one
	if (sm_pMusBGM != nullptr)
	{
		Mix_FreeMusic(sm_pMusBGM);
	}

	//load new one
	sm_pMusBGM = Mix_LoadMUS(filename);
	if (sm_pMusBGM == nullptr)
	{
		std::cout << "LoadBGMFromFile() couldn't load music from file." << std::endl;
	}
}

void AudioEngine::LoadBGMFromMem(void* mem, unsigned int size)
{
	auto rw = SDL_RWFromMem(mem, size);
	if (rw == nullptr)
	{
		std::cout << "LoadBGMFromMem() couldn't load rwops from mem." << std::endl;
		return;
	}
	sm_pMusBGM = Mix_LoadMUS_RW(rw, 1);//free rw
	if (sm_pMusBGM == nullptr)
	{
		std::cout << "LoadBGMFromMem() couldn't load music from rwops." << std::endl;
		return;//?
	}
}

void AudioEngine::BGMVolume(int new_volume)
{
	if (new_volume < 0 || new_volume>128)
	{
		std::cout << "BGMVolume() new volume out of range.It have been set to 64." << std::endl;
		Mix_VolumeMusic(64);
		sm_nBGMVolume = 64;
	}
	else
	{
		Mix_VolumeMusic(new_volume);
		sm_nBGMVolume = new_volume;
	}
}

int AudioEngine::BGMVolume()
{
	return sm_nBGMVolume;
}

void AudioEngine::SetFinishedBGMCB(void(*cb)())
{
	Mix_HookMusicFinished(cb);
}

SEAudio* AudioEngine::LoadSEAudioFromFile(const char* filename)
{
	Mix_Chunk* pChunk = Mix_LoadWAV(filename);
	if (pChunk == nullptr)
	{
		std::cout << "LoadSEAudioFromFile() couldn't load from file." << std::endl;
		return new SEAudio(nullptr, 0);
	}

	int freeChannel = __FindEmptyChannel();
	if (freeChannel == -1)
	{
		std::cout << "LoadSEAudioFromFile() could't find free channel." << std::endl;
		Mix_FreeChunk(pChunk);
		return new SEAudio(nullptr, 0);
	}

	auto obj = new SEAudio(pChunk, freeChannel);
	if (obj == nullptr)
	{
		std::cout << "LoadSEAudioFromFile() could't allocate mem for seaudio." << std::endl;
		Mix_FreeChunk(pChunk);
		return new SEAudio(nullptr, 0);
	}

	return obj;
}

SEAudio* AudioEngine::LoadSEAudioFromMem(void* mem, unsigned int size)
{
	auto rw = SDL_RWFromMem(mem, size);
	if (rw == nullptr)
	{
		std::cout << "LoadSEAudioFromFile() couldn't load from mem." << std::endl;
		return new SEAudio(nullptr, 0);
	}

	Mix_Chunk* pChunk = Mix_LoadWAV_RW(rw, 1);//free rw
	if (pChunk == nullptr)
	{
		std::cout << "LoadSEAudioFromFile() couldn't load from rw." << std::endl;
		return new SEAudio(nullptr, 0);
	}

	int freeChannel = __FindEmptyChannel();
	if (freeChannel == -1)
	{
		std::cout << "LoadSEAudioFromFile() could't find free channel." << std::endl;
		Mix_FreeChunk(pChunk);
		return new SEAudio(nullptr, 0);
	}

	auto obj = new SEAudio(pChunk, freeChannel);
	if (obj == nullptr)
	{
		std::cout << "LoadSEAudioFromFile() could't allocate mem for seaudio." << std::endl;
		Mix_FreeChunk(pChunk);
		return new SEAudio(nullptr, 0);
	}

	return obj;
}

void AudioEngine::FreeSEAudio(SEAudio* se)
{
	sm_bChannelUsed[se->GetChannel()] = false;
	se->FreeChunk();
	delete se;
	se = nullptr;
}

void AudioEngine::StopAllSEAudio()
{
	Mix_HaltChannel(-1);
}

int AudioEngine::__FindEmptyChannel()
{
	int freeChannel = -1;
	for (int i = 0; i != 64; i++)
	{
	}
	return 0;
}

SEAudio::SEAudio(Mix_Chunk* pChunk, int channel)
{
	m_nChannel = channel;
	m_nPosAngle = 0;
	m_nPosDistance = 0;
	m_pChunk = pChunk;
	//SetPosition(m_nPosAngle,m_nPosDistance);
}

void SEAudio::Play()
{
	if (m_pChunk != nullptr)
	{
		if (!Mix_Playing(m_nChannel))
		{
			Mix_PlayChannel(m_nChannel, m_pChunk, 0);
		}
	}
	else
	{
		std::cout << "SEAudio::Play() m_pChunk is null pointer." << std::endl;
	}
}

void SEAudio::Pause()
{
	Mix_Pause(m_nChannel);
}

void SEAudio::Resume()
{
	Mix_Resume(m_nChannel);
}

void SEAudio::Stop()
{
	Mix_HaltChannel(m_nChannel);
}

void SEAudio::SetPosition(int angle, int distance)
{
	Mix_SetPosition(m_nChannel, angle, distance);
	m_nPosAngle = angle;
	m_nPosDistance = distance;
}

int SEAudio::GetPositionAngle()
{
	return m_nPosAngle;
}

int SEAudio::GetPositionDistance()
{
	return m_nPosDistance;
}

int SEAudio::GetChannel()
{
	return m_nChannel;
}

void SEAudio::SetFinishedCB(void(*cb)(int channel_useless))
{
	Mix_ChannelFinished(cb);
}

void SEAudio::FreeChunk()
{
	Mix_FreeChunk(m_pChunk);
	m_pChunk = nullptr;
}

GraphUnit::GraphUnit(SDL_Renderer* pRen, SDL_Texture* pTex)
{
	m_pRen = pRen;
	m_pTex = pTex;
	m_rtDst = { 0,0,0,0 };
}

GraphUnit::~GraphUnit()
{
	SDL_DestroyTexture(m_pTex);
}

void GraphUnit::Rect(const SDL_Rect& rect_new)
{
	if (rect_new.w > 0 && rect_new.h > 0)
	{
		m_rtDst = rect_new;
	}
	else
	{
		m_rtDst.x = rect_new.x;
		m_rtDst.y = rect_new.y;
	}
}

const SDL_Rect& GraphUnit::Rect()
{
	return m_rtDst;
}

void GraphUnit::RenderCopy()
{
	SDL_RenderCopy(m_pRen, m_pTex, nullptr, &m_rtDst);
}

void GraphUnitPack::RenderCopy(unsigned int index)
{
	if (index >= m_vec_SrcRect.size())
	{
		throw std::string("GraphUnitPack::RenderCopy(unsigned int index)\nindex超出范围");
	}
	SDL_RenderCopy(m_pRen, m_pTex, &m_vec_SrcRect[index], &m_rtDst);
}

void GraphUnitPack::UnitSize(int size)
{
	if (size <= 0)
	{
		throw std::string("GraphUnitPack::UnitSize(unsigned int size)\nsize不合法");
	}
	m_nUnitSize = size;
	_ArrangeSrcRect();
}

unsigned int GraphUnitPack::UnitSize()
{
	return m_nUnitSize;
}

void GraphUnitPack::_ArrangeSrcRect()
{
	m_vec_SrcRect.clear();
	int x_surplus = m_ptPackSize.x % m_nUnitSize;
	int y_surplus = m_ptPackSize.y % m_nUnitSize;
	if (x_surplus > 0 || y_surplus > 0)
	{
		//只是输出警告
		std::cout << "GraphUnitPack::_ArrangeSrcRect() size not fit\n";
	}
	SDL_Rect tmp;
	for (int y = 0; y < m_ptPackSize.y - y_surplus; y += m_nUnitSize)
	{
		for (int x = 0; x < m_ptPackSize.x - x_surplus; x += m_nUnitSize)
		{
			tmp = { x,y,m_nUnitSize,m_nUnitSize };
			m_vec_SrcRect.push_back(tmp);
		}
	}
}

Controller::Controller(SDL_Event* msg)
{
	if (msg == nullptr)
	{
		throw "Controller::Controller() msg null";
	}
	m_pMsg = msg;
}

void Controller::Handle()
{
	for (auto& p : m_map_key_state)
	{
		switch (p.second)
		{
		case KS_HOLDING:
			if (m_pMsg->type == SDL_KEYUP && m_pMsg->key.keysym.sym == p.first)
			{
				p.second = KS_UP;
			}
			break;
		case KS_DOWN:
			if (m_pMsg->type == SDL_KEYUP && m_pMsg->key.keysym.sym == p.first)
			{
				p.second = KS_UP;
			}
			else
			{
				p.second = KS_HOLDING;
			}
			break;
		case KS_UP:
			if (m_pMsg->type == SDL_KEYDOWN && m_pMsg->key.keysym.sym == p.first)
			{
				p.second = KS_DOWN;
			}
			else
			{
				p.second = KS_NONE;
			}
			break;
		case KS_NONE:
			if (m_pMsg->type == SDL_KEYDOWN && m_pMsg->key.keysym.sym == p.first)
			{
				p.second = KS_DOWN;
			}
			else if (m_pMsg->type == SDL_KEYUP && m_pMsg->key.keysym.sym == p.first)
			{
				p.second = KS_UP;
			}
			break;
		default:
			break;
		}
	}

	if (m_map_key_state.count(m_pMsg->key.keysym.sym) == 0)
	{
		switch (m_pMsg->type)
		{
		case SDL_KEYDOWN:
			m_map_key_state[m_pMsg->key.keysym.sym] = KS_DOWN;
			break;
		case SDL_KEYUP:
			m_map_key_state[m_pMsg->key.keysym.sym] = KS_UP;
			break;
		default:
			break;
		}
	}
}

bool Controller::IsDowning(SDL_Keycode code)
{
	if (m_map_key_state.count(code) != 0)
	{
		if (m_map_key_state[code] == KS_DOWN)
		{
			return true;
		}
	}
	return false;
}

bool Controller::IsHolding(SDL_Keycode code)
{
	if (m_map_key_state.count(code) != 0)
	{
		if (m_map_key_state[code] == KS_HOLDING)
		{
			return true;
		}
	}
	return false;
}

bool Controller::IsUping(SDL_Keycode code)
{
	if (m_map_key_state.count(code) != 0)
	{
		if (m_map_key_state[code] == KS_UP)
		{
			return true;
		}
	}
	return false;
}

void Controller::__PushEmptyEvent()
{
	static auto user_event_id = SDL_RegisterEvents(1);
	SDL_Event emptyEvent;
	emptyEvent.type = user_event_id;
	SDL_PushEvent(&emptyEvent);
}
