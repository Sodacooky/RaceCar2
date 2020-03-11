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


//һ��������ʾ�ĵ�Ԫ��GraphEngine����
class GraphUnit
{
public:
	GraphUnit(SDL_Renderer* pRen, SDL_Texture* pTex);
	~GraphUnit();
	//���w��h��0���Сʱ��ֻ�ı�λ��
	void Rect(const SDL_Rect& rect_new);
	const SDL_Rect& Rect();

	virtual void RenderCopy();
protected:
	SDL_Rect m_rtDst;
	SDL_Texture* m_pTex;
	SDL_Renderer* m_pRen;
};

//һϵ��GraphUnit��Pack��GraphEngine����
class GraphUnitPack : public GraphUnit
{
public:
	GraphUnitPack(SDL_Renderer* pRen, SDL_Texture* pTex, const SDL_Point& pack_size)
		:GraphUnit(pRen, pTex)
	{
		m_ptPackSize = pack_size;
		_ArrangeSrcRect();
	}
	//index Ϊ Ҫcopy��tex���еı��
	virtual void RenderCopy(unsigned int index);
	//���ñ߳���Ĭ��Ϊ16
	//���ñ߳����ÿ��src_rect���¼���
	void UnitSize(int size);
	//�߳�
	unsigned int UnitSize();
protected:
	void _ArrangeSrcRect();
protected:
	//�±��Ӧpack�ı��
	std::vector<SDL_Rect> m_vec_SrcRect;
	//
	int m_nUnitSize = 16;
	//
	SDL_Point m_ptPackSize;
};

//֧�������ֱ�������ͼ������
class GraphEngine
{
public:
	//��ʼ�����ϵͳ��������ڼ�����Դǰ����
	static bool Init();

	//�ͷſռ䲢�ر�
	static void Quit();

	//�ı䴰�ڵķŴ����������2�ı���
	static void ChangeWinRect(const SDL_Rect& rtWin);

	//�ı䴰�ڱ���
	static void ChangeWinTitle(const char* utf8_title);

	//����buff���������buff
	static void RenderPresent();

	//���buff
	static void RenderClear();

	//���ļ��м���graphunit������Ҫ����Ⱦʱ��Ҫ��������ʱ��ǵ�����
	static GraphUnit* LoadUnitFromFile(const char* filename, bool use_key = false,
									   const SDL_Color& key = { 0,255,0,255 });

	//���ڴ����graphunit,����Ҫ����Ⱦʱ��Ҫ��������ʱ��ǵ�����
	static GraphUnit* LoadUnitFromMem(void* mem, unsigned int mem_size,
									  bool use_key = false,
									  const SDL_Color& key = { 0,255,0,255 });

	//��һ���ַ���ת��ΪGraphUnit
	//��һ����������һ���ȱȵĻ��ƴ�С����Ϊ16
	static GraphUnit* LoadUnitFromAnsiStr(SDL_Point& size, const char* ansi_cstr,
										  const SDL_Color& cl={255,255,255});

	//��һ���ַ���ת��ΪGraphUnit
	//��һ����������һ���ȱȵĻ��ƴ�С����Ϊ16
	static GraphUnit* LoadUnitFromUtf8Str(SDL_Point& size, const char* utf8_cstr,
										  const SDL_Color& cl = { 255,255,255 });

	//���ļ��м���graphunit������Ҫ����Ⱦʱ��Ҫ��������ʱ��ǵ�����
	//����ļ����зֳ�16����С�����ΰ�˳����
	static GraphUnitPack*
		LoadSqUnitPackFromFile(const char* filename, bool use_key = false,
							   const SDL_Color& key = { 0,255,0,255 });

	//���ڴ��м���graphunit������Ҫ����Ⱦʱ��Ҫ��������ʱ��ǵ�����
	//����ļ����зֳ�16����С�����ΰ�˳����
	static GraphUnitPack*
		LoadSqUnitPackFromMem(void* mem, unsigned int mem_size,
							  bool use_key = false,
							  const SDL_Color& key = { 0,255,0,255 });

	static void FreeUnit(GraphUnit* gu);

	static void FreeUnitPack(GraphUnitPack* gup);

	//������ʹ��
	static SDL_Window* GetSDLWin();
	static SDL_Renderer* GetSDLRen();
private:
	static SDL_Window* sm_pWin;
	static SDL_Renderer* sm_pRen;

	static SDL_Rect sm_rtWinRect;

	static TTF_Font* sm_pFont;
};

//��Ч��ͨ��AudioEngine����
class SEAudio
{
public:
	SEAudio(Mix_Chunk* pChunk, int channel);
	void Play();
	void Pause();
	void Resume();
	void Stop();
	//��ǰ����ʼ��˳ʱ��360�ȣ��������ֵ��255
	//�ұ�90�ȣ����270�ȣ�ǰ��0��
	//����ͨ������ľ�������������
	void SetPosition(int angle = 0, int distance = 0);
	//
	int GetPositionAngle();
	//
	int GetPositionDistance();
	//
	int GetChannel();
	//������Ч����ʱ�Ļص�
	void SetFinishedCB(void(*cb)(int channel_useless) = nullptr);
	//�ͷ�chunk�ڴ�
	void FreeChunk();
private:
	Mix_Chunk* m_pChunk;
	int m_nChannel;
	int m_nPosAngle;
	int m_nPosDistance;
};

//��Ƶ����
class AudioEngine
{
public:
	static bool Init();
	static void Quit();

	//��ʼ���ű������֣�����Ѿ��ڲ��ţ���ʲô������
	static void PlayBGM(int fade_ms = 0);
	//��λ(����ͷ)��������
	static void RewindBGM();
	//ֹͣ���ű�������
	static void StopBGM(int fade_ms = 0);
	//��ͣ���ű�������
	static void PauseBGM();
	//����ͣλ�ü������ű�������
	static void ResumeBGM();
	//���ر������ִ��ļ���������ɵı�������
	static void LoadBGMFromFile(const char* filename);
	//���ر������ִ��ڴ棬������ɵı�������
	static void LoadBGMFromMem(void* mem, unsigned int size);
	//���ñ������ֵ�����0-128
	static void BGMVolume(int new_volume);
	//��ǰ������������
	static int BGMVolume();
	//���ñ������ֽ�����Ļص�����
	//��ͣ���ᴥ��
	static void SetFinishedBGMCB(void(*cb)() = nullptr);

	//����һ��SE������Ҫ��FreeSEAudio���ͷ����ָ����ڴ�
	//��������Խ�chunk�Լ��ͷŵ������ͷŵ�Ҳ��
	static SEAudio* LoadSEAudioFromFile(const char* filename);
	////����һ��SE������Ҫ��FreeSEAudio���ͷ����ָ����ڴ�
	//��������Խ�chunk�Լ��ͷŵ������ͷŵ�Ҳ��
	static SEAudio* LoadSEAudioFromMem(void* mem, unsigned int size);
	//�ͷ�SEAudio
	static void FreeSEAudio(SEAudio* se);
	//ֹͣ����SEAudio
	static void StopAllSEAudio();
private:
	static Mix_Music* sm_pMusBGM;
	static int sm_nBGMVolume;
	//true��ʾ�ѱ�ʹ��
	static bool sm_bChannelUsed[64];
private:
	//-1 none free channel
	static int __FindEmptyChannel();
};

//���������������װ������������
class RandomEngine
{
public:
	static void Init();
	static int UniformRange(int min, int max);
	static int NormalRange(int mu, int sigma);
	static bool Half();
	//ʹ��sdl��gettick�������ã����sdlδ��ʼ�����ܻ�������
	static void SetNewSeed();
	//�����µ�����
	static void SetNewSeed(unsigned int seed);
private:
	static std::default_random_engine sm_dre;
};

class Controller
{
public:
	Controller(SDL_Event* msg);
	//�ô���ȥ��msg����һ�¼���ʱ��
	void Handle();

	//������Ƿ��ǰ���˲��
	bool IsDowning(SDL_Keycode code);
	//������Ƿ񱻰���
	bool IsHolding(SDL_Keycode code);
	//������Ƿ����ɿ�˲��
	bool IsUping(SDL_Keycode code);
private:
	enum KeyState
	{
		KS_NONE,//�����û������Ҳû��
		KS_UP,//��̧��˲��
		KS_HOLDING,//�����ڱ�����
		KS_DOWN//������˲��
	};
	SDL_Event* m_pMsg;
	std::map<SDL_Keycode, KeyState> m_map_key_state;

	void __PushEmptyEvent();
};