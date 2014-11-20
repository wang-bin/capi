#include "sdl_api.h"
#include "capi.h"

namespace sdl {

//DEFINE_DLL_INSTANCE_N("sdl", "SDL", "SDL32", "SDL-1.2", NULL)
static const char* sdl_names[] = { "SDL", "SDL32", "SDL-1.2", NULL };
CAPI_BEGIN_DLL(sdl_names)
CAPI_DEFINE_M_RESOLVER(1, int, SDLCALL, SDL_Init, Uint32)
CAPI_DEFINE_M_RESOLVER(2, void, SDLCALL, SDL_WM_SetCaption, const char*, const char*)
CAPI_DEFINE_M_RESOLVER(1, int, SDLCALL, SDL_PollEvent, SDL_Event*)
CAPI_DEFINE_M_RESOLVER(4, SDL_Surface*, SDLCALL, SDL_SetVideoMode, int, int, int, Uint32)
CAPI_DEFINE_M_RESOLVER(0, void, SDLCALL, SDL_Quit)
CAPI_END_DLL()

api::api() : dll(new api_dll()) {
    qDebug("capi::version: %s build %s", capi::version::name, capi::version::build());
}
api::~api() { delete dll;}

CAPI_DEFINE(1, int, SDL_Init, Uint32)
CAPI_DEFINE(2, void, SDL_WM_SetCaption, const char*, const char*)
CAPI_DEFINE(1, int, SDL_PollEvent, SDL_Event*)
CAPI_DEFINE(4, SDL_Surface*, SDL_SetVideoMode, int, int, int, Uint32)
CAPI_DEFINE(0, void, SDL_Quit)

} //namespace sdl
