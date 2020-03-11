#pragma once


#include <iostream>
#include <vector>
#include <random>
#include <functional>
#include <Windows.h>
#include <map>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_mixer.h"

#pragma comment (lib,"SDL2.lib")
#pragma comment (lib,"SDL2main.lib")
#pragma comment (lib,"SDL2_image.lib")
#pragma comment (lib,"SDL2_ttf.lib")
#pragma comment (lib,"SDL2_mixer.lib")


//一个可以显示的单元，GraphEngine创建
class GraphUnit
{
public:
	GraphUnit(SDL_Renderer* pRen, SDL_Texture* pTex);
	~GraphUnit();
	//如果w或h是0或更小时，只改变位置
	void Rect(const SDL_Rect& rect_new);
	const SDL_Rect& Rect();

	virtual void RenderCopy();
protected:
	SDL_Rect m_rtDst;
	SDL_Texture* m_pTex;
	SDL_Renderer* m_pRen;
};

//一系列GraphUnit的Pack，GraphEngine创建
class GraphUnitPack : public GraphUnit
{
public:
	GraphUnitPack(SDL_Renderer* pRen, SDL_Texture* pTex, const SDL_Point& pack_size)
		:GraphUnit(pRen, pTex)
	{
		m_ptPackSize = pack_size;
		_ArrangeSrcRect();
	}
	//index 为 要copy的tex包中的编号
	virtual void RenderCopy(unsigned int index);
	//设置边长，默认为16
	//设置边长会对每个src_rect重新计算
	void UnitSize(int size);
	//边长
	unsigned int UnitSize();
protected:
	void _ArrangeSrcRect();
protected:
	//下标对应pack的编号
	std::vector<SDL_Rect> m_vec_SrcRect;
	//
	int m_nUnitSize = 16;
	//
	SDL_Point m_ptPackSize;
};

//支持网格和直接坐标的图像引擎
class GraphEngine
{
public:
	//初始化这个系统，请务必在加载资源前调用
	static bool Init();

	//释放空间并关闭
	static void Quit();

	//改变窗口的放大倍数，最好是2的倍数
	static void ChangeWinRect(const SDL_Rect& rtWin);

	//改变窗口标题
	static void ChangeWinTitle(const char* utf8_title);

	//呈现buff，但不清空buff
	static void RenderPresent();

	//清空buff
	static void RenderClear();

	//从文件中加载graphunit，在需要被渲染时不要清理，不用时请记得清理
	static GraphUnit* LoadUnitFromFile(const char* filename, bool use_key = false,
									   const SDL_Color& key = { 0,255,0,255 });

	//从内存加载graphunit,在需要被渲染时不要清理，不用时请记得清理
	static GraphUnit* LoadUnitFromMem(void* mem, unsigned int mem_size,
									  bool use_key = false,
									  const SDL_Color& key = { 0,255,0,255 });

	//将一个字符串转换为GraphUnit
	//第一个参数返回一个等比的绘制大小，高为16
	static GraphUnit* LoadUnitFromAnsiStr(SDL_Point& size, const char* ansi_cstr,
										  const SDL_Color& cl={255,255,255});

	//将一个字符串转换为GraphUnit
	//第一个参数返回一个等比的绘制大小，高为16
	static GraphUnit* LoadUnitFromUtf8Str(SDL_Point& size, const char* utf8_cstr,
										  const SDL_Color& cl = { 255,255,255 });

	//从文件中加载graphunit，在需要被渲染时不要清理，不用时请记得清理
	//这个文件被切分成16长的小正方形按顺序存放
	static GraphUnitPack*
		LoadSqUnitPackFromFile(const char* filename, bool use_key = false,
							   const SDL_Color& key = { 0,255,0,255 });

	//从内存中加载graphunit，在需要被渲染时不要清理，不用时请记得清理
	//这个文件被切分成16长的小正方形按顺序存放
	static GraphUnitPack*
		LoadSqUnitPackFromMem(void* mem, unsigned int mem_size,
							  bool use_key = false,
							  const SDL_Color& key = { 0,255,0,255 });

	static void FreeUnit(GraphUnit* gu);

	static void FreeUnitPack(GraphUnitPack* gup);

	//不建议使用
	static SDL_Window* GetSDLWin();
	static SDL_Renderer* GetSDLRen();
private:
	static SDL_Window* sm_pWin;
	static SDL_Renderer* sm_pRen;

	static SDL_Rect sm_rtWinRect;

	static TTF_Font* sm_pFont;
};

//音效，通过AudioEngine创建
class SEAudio
{
public:
	SEAudio(Mix_Chunk* pChunk, int channel);
	void Play();
	void Pause();
	void Resume();
	void Stop();
	//正前方开始，顺时针360度，距离最大值是255
	//右边90度，左边270度，前方0度
	//建议通过这里的距离来控制音量
	void SetPosition(int angle = 0, int distance = 0);
	//
	int GetPositionAngle();
	//
	int GetPositionDistance();
	//
	int GetChannel();
	//设置音效结束时的回调
	void SetFinishedCB(void(*cb)(int channel_useless) = nullptr);
	//释放chunk内存
	void FreeChunk();
private:
	Mix_Chunk* m_pChunk;
	int m_nChannel;
	int m_nPosAngle;
	int m_nPosDistance;
};

//音频引擎
class AudioEngine
{
public:
	static bool Init();
	static void Quit();

	//开始播放背景音乐，如果已经在播放，则什么都不做
	static void PlayBGM(int fade_ms = 0);
	//复位(到开头)背景音乐
	static void RewindBGM();
	//停止播放背景音乐
	static void StopBGM(int fade_ms = 0);
	//暂停播放背景音乐
	static void PauseBGM();
	//从暂停位置继续播放背景音乐
	static void ResumeBGM();
	//加载背景音乐从文件，会清除旧的背景音乐
	static void LoadBGMFromFile(const char* filename);
	//加载背景音乐从内存，会清除旧的背景音乐
	static void LoadBGMFromMem(void* mem, unsigned int size);
	//设置背景音乐的音量0-128
	static void BGMVolume(int new_volume);
	//当前背景音乐音量
	static int BGMVolume();
	//设置背景音乐结束后的回调函数
	//暂停不会触发
	static void SetFinishedBGMCB(void(*cb)() = nullptr);

	//创建一个SE，你需要用FreeSEAudio来释放这个指针的内存
	//但是你可以将chunk自己释放掉，不释放掉也可
	static SEAudio* LoadSEAudioFromFile(const char* filename);
	////创建一个SE，你需要用FreeSEAudio来释放这个指针的内存
	//但是你可以将chunk自己释放掉，不释放掉也可
	static SEAudio* LoadSEAudioFromMem(void* mem, unsigned int size);
	//释放SEAudio
	static void FreeSEAudio(SEAudio* se);
	//停止所有SEAudio
	static void StopAllSEAudio();
private:
	static Mix_Music* sm_pMusBGM;
	static int sm_nBGMVolume;
	//true表示已被使用
	static bool sm_bChannelUsed[64];
private:
	//-1 none free channel
	static int __FindEmptyChannel();
};

//随机数产生器，封装起来避免问题
class RandomEngine
{
public:
	static void Init();
	static int UniformRange(int min, int max);
	static int NormalRange(int mu, int sigma);
	static bool Half();
	//使用sdl的gettick重新设置，如果sdl未初始化可能会有问题
	static void SetNewSeed();
	//设置新的种子
	static void SetNewSeed(unsigned int seed);
private:
	static std::default_random_engine sm_dre;
};

class Controller
{
public:
	Controller(SDL_Event* msg);
	//用传进去的msg处理一下键盘时间
	void Handle();

	//这个键是否被是按下瞬间
	bool IsDowning(SDL_Keycode code);
	//这个键是否被按着
	bool IsHolding(SDL_Keycode code);
	//这个键是否是松开瞬间
	bool IsUping(SDL_Keycode code);
private:
	enum KeyState
	{
		KS_NONE,//这个键没被按，也没事
		KS_UP,//键抬起瞬间
		KS_HOLDING,//键正在被按着
		KS_DOWN//键按下瞬间
	};
	SDL_Event* m_pMsg;
	std::map<SDL_Keycode, KeyState> m_map_key_state;

	void __PushEmptyEvent();
};