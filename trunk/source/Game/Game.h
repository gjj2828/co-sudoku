#ifndef __GAME_H__
#define __GAME_H__

#include <IGame.h>
#include <IPuzzleSystem.h>
#include <IRenderSystem.h>

class CGame: public IGame
{
public:
    CGame();
    virtual int                 Init(HINSTANCE hInstance);
    virtual void                Run();
    virtual void                Release();
    virtual GlobalEnviroment*   GetEnv() {return &m_env;}

private:
    enum EModule
    {
        EMODULE_MIN,
        EMODULE_PUZZLE  = EMODULE_MIN,
        EMODULE_RENDER,
        EMODULE_MAX,
    };

    HINSTANCE           m_hInstance;
    HWND                m_hWnd;
    HBRUSH              m_hBkBrush;
    HMODULE             m_hModules[EMODULE_MAX];
    GlobalEnviroment    m_env;

    int LoadDll();
    int InitWindow();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};

#endif // __GAME_H__
