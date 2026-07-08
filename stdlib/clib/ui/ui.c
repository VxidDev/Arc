#include "../../../include/object.h"
#include "../../../include/utils.h"

#include <stdlib.h>

#include <SDL3/SDL.h>

static SDL_Event globalEv;

// create_frame(width: int, height: int, name: string) -> int64_t
Object* arcUI_create_frame(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // width 
  if (err) return err;

  err = enforceType(args[1], OBJ_NUMBER_INT, 2); // height 
  if (err) return err;
  
  err = enforceType(args[2], OBJ_STRING, 3); // name
  if (err) return err;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    char buf[1024];

    snprintf(buf, sizeof(buf), "SDL initialization failed: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }
  
  SDL_Window* window = SDL_CreateWindow(
    ((String*)args[2])->value,
    ((Number*)args[0])->as.i,
    ((Number*)args[1])->as.i,
    SDL_WINDOW_RESIZABLE
  );

  if (!window) {
    SDL_Quit();

    char buf[1024];

    snprintf(buf, sizeof(buf), "Window creation failed: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

  if (!renderer) {
    SDL_DestroyWindow(window);
    SDL_Quit();
    return (Object*)initProgramError("Failed to initialize renderer.");
  }
  
  Object *tmp[2] = { (Object*)initInt((int64_t)(uintptr_t)window), (Object*)initInt((int64_t)(uintptr_t)renderer) };
  List* out = initList(tmp, 2, 2);

  return (Object*)out;
}

Object* arcUI_delay(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  int64_t delay = ((Number*)args[0])->as.i;

  SDL_Delay(delay);

  return (Object*)initInt(1);
}

Object* arcUI_render_present(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // renderer 
  if (err) return err;
  
  SDL_RenderPresent((void*)(uintptr_t)((Number*)args[0])->as.i);

  return (Object*)initInt(1);
}


Object* arcUI_quit(Object** args, size_t argCount) {
  (void)argCount;
  (void)args;
  
  SDL_Quit();

  return (Object*)initInt(1);
}

Object* arcUI_set_render_draw_color(Object** args, size_t argCount) {
  (void)argCount;
  
  for (size_t i = 0; i < 5; i++) {
    Object* err = enforceType(args[i], OBJ_NUMBER_INT, i + 1);
    if (err) return err;
  }
  
  void* renderer = (void*)(uintptr_t)((Number*)args[0])->as.i;

  int64_t r = ((Number*)args[1])->as.i;

  if (r < 0 || r > 255) {
    return (Object*)initProgramError("Color channel 'R' must be between 0 and 255.");
  }

  int64_t g = ((Number*)args[2])->as.i;

  if (g < 0 || g > 255) {
    return (Object*)initProgramError("Color channel 'G' must be between 0 and 255.");
  }

  int64_t b = ((Number*)args[3])->as.i;

  if (b < 0 || b > 255) {
    return (Object*)initProgramError("Color channel 'B' must be between 0 and 255.");
  }

  int64_t a = ((Number*)args[4])->as.i;

  if (a < 0 || a > 255) {
    return (Object*)initProgramError("Color channel 'A' must be between 0 and 255.");
  }

  bool ok = SDL_SetRenderDrawColor(renderer, r, g, b, a);

  if (!ok) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "Failed to set draw color: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  return (Object*)initInt(1);
}

Object* arcUI_render_clear(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  bool ok = SDL_RenderClear((void*)(uintptr_t)((Number*)args[0])->as.i);

  if (!ok) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "Failed to clear renderer: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  return (Object*)initInt(1);
}

Object* arcUI_pump_events(Object** args, size_t argCount) {
  (void)argCount;
  (void)args;

  SDL_PumpEvents();

  return (Object*)initInt(1);
}

Object* arcUI_poll_event(Object** args, size_t argCount) {
  (void)argCount;
  (void)args;

  if (SDL_PollEvent(&globalEv)) {
    return (Object*)initInt((int64_t)(uintptr_t)&globalEv);
  }

  return (Object*)initInt(0);
}
