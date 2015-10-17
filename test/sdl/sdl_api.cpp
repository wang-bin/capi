#define SDL_CAPI_BUILD
#define DEBUG_LOAD
//#define CAPI_IS_LAZY_RESOLVE 0 //define it will resolve all symbols in constructor
#ifndef CAPI_LINK_SDL
#include "capi.h"
#endif //CAPI_LINK_SDL
#include "sdl_api.h" //include last because sdl.h was in namespace capi to avoid covering types later

namespace sdl {
#ifdef CAPI_LINK_SDL
api::api(){dll=0;}
api::~api(){}
bool api::loaded() const { return true;}
#else
static const char* sdl_names[] = { "SDL", "SDL32", "sdl", "SDL-1.2", NULL };
static const int sdl_vers[] = { ::capi::NoVersion, 2, 1, ::capi::EndVersion };
CAPI_BEGIN_DLL_VER(sdl_names, sdl_vers, ::capi::dso) // you can also use QLibrary or your custom library resolver instead of ::capi::dso
CAPI_DEFINE_M_ENTRY(int, SDLCALL, SDL_Init, CAPI_ARG1(Uint32))
CAPI_DEFINE_M_ENTRY(void, SDLCALL, SDL_WM_SetCaption, CAPI_ARG2(const char*, const char*))
CAPI_DEFINE_M_ENTRY(int, SDLCALL, SDL_PollEvent, CAPI_ARG1(SDL_Event*))
CAPI_DEFINE_M_ENTRY(SDL_Surface*, SDLCALL, SDL_SetVideoMode, CAPI_ARG4(int, int, int, Uint32))
CAPI_DEFINE_M_ENTRY(void, SDLCALL, SDL_Quit, CAPI_ARG0())
CAPI_END_DLL()
CAPI_DEFINE_DLL
CAPI_DEFINE(int, SDL_Init, CAPI_ARG1(Uint32))
CAPI_DEFINE(void, SDL_WM_SetCaption, CAPI_ARG2(const char*, const char*))
CAPI_DEFINE(int, SDL_PollEvent, CAPI_ARG1(SDL_Event*))
CAPI_DEFINE(SDL_Surface*, SDL_SetVideoMode, CAPI_ARG4(int, int, int, Uint32))
CAPI_DEFINE(void, SDL_Quit, CAPI_ARG0())
#endif //CAPI_LINK_SDL
} //namespace sdl
