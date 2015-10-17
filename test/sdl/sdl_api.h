#ifndef CAPI_SDL_H
#define CAPI_SDL_H

extern "C" {
#include <SDL/SDL.h>
}
// no need to include the C header if only functions declared there
#ifndef CAPI_LINK_SDL
namespace sdl { //need a unique namespace
namespace capi {
#else
extern "C" {
#endif
#include "SDL/SDL.h" //// we need some types define there. otherwise we can remove this
#ifndef CAPI_LINK_SDL
}
#endif
}

namespace sdl { //need a unique namespace
#ifndef CAPI_LINK_SDL // avoid ambiguous in sdl_api.cpp
using namespace capi;
#endif
class api_dll; //must use this name
class api //must use this name
{
    api_dll *dll;
public:
    api();
    virtual ~api();
    virtual bool loaded() const;
#if !defined(CAPI_LINK_SDL) && !defined(SDL_CAPI_NS)
    int SDL_Init(Uint32 flags);
    void SDL_Quit(void);
    int SDL_PollEvent(SDL_Event *event);
    SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
    void SDL_WM_SetCaption(const char *title, const char *icon);
#endif
};
} //namespace sdl
#ifndef SDL_CAPI_BUILD
#ifdef SDL_CAPI_NS
using namespace sdl::capi;
#else
using namespace sdl;
#endif
#endif //SDL_CAPI_BUILD
#endif // SDL_API_H
