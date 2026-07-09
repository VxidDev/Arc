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
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    char buf[1024];

    snprintf(buf, sizeof(buf), "Window creation failed: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

  if (!renderer) {
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
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
  
  SDL_QuitSubSystem(SDL_INIT_VIDEO);

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

double __get_double(Number* num) {
  if (num->base.type == OBJ_NUMBER_INT) return (double)num->as.i;
  if (num->base.type == OBJ_NUMBER_FLOAT) return (double)num->as.f;
  return 0.0;
}

Object* arcUI_rect(Object** args, size_t argCount) {
  (void)argCount;
  
  for (size_t i = 0; i < 4; i++) {
    if (args[i]->type != OBJ_NUMBER_INT && args[i]->type != OBJ_NUMBER_FLOAT) {
      char buf[256];
      snprintf(buf, sizeof(buf), "Expected argument %zu to be object of type 'int' or 'float', received '%s'", i + 1, typeofobj(args[i]));
    }
  }

  double x = __get_double((Number*)args[0]);
  double y = __get_double((Number*)args[1]);
  double width = __get_double((Number*)args[2]);
  double height = __get_double((Number*)args[3]);
  
  SDL_FRect *rect = malloc(sizeof(SDL_FRect));

  if (!rect) {
    return (Object*)initProgramError("Failed to create rect object (Out of memory)");
  }

  *rect = (SDL_FRect){ x, y, width, height };

  return (Object*)initInt((int64_t)(uintptr_t)rect);
}

Object* arcUI_fill_rect(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // renderer 
  if (err) return err;

  err = enforceType(args[1], OBJ_NUMBER_INT, 2); // rect ptr
  if (err) return err;

  void* renderer = (void*)(uintptr_t)((Number*)args[0])->as.i;
  void* rectPtr = (void*)(uintptr_t)((Number*)args[1])->as.i;

  if (!SDL_RenderFillRect(renderer, rectPtr)) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "Failed to fill rect: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  } 

  return (Object*)initInt(1);
}

Object* arcUI_get_mouse_info(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // event ptr 
  if (err) return err;

  SDL_Event ev = *(SDL_Event*)(uintptr_t)((Number*)args[0])->as.i;

  if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN || ev.type == SDL_EVENT_MOUSE_BUTTON_UP) {
    Object* x = (Object*)initFloat(ev.button.x);
    Object* y = (Object*)initFloat(ev.button.y);
    Object* button = (Object*)initInt(ev.button.button == SDL_BUTTON_LEFT ? 0 : 1);
    Object* status = (Object*)initInt(ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1 : 0);

    return (Object*)initList((Object*[]){x, y, button, status}, 4, 4);
  }

  return (Object*)initNull();
}

Object* arcUI_point(Object** args, size_t argCount) {
  (void)argCount;
  
  for (size_t i = 0; i < 2; i++) {
    if (args[i]->type != OBJ_NUMBER_INT && args[i]->type != OBJ_NUMBER_FLOAT) {
      char buf[256];
      snprintf(buf, sizeof(buf), "Expected argument %zu to be object of type 'int' or 'float', received '%s'", i + 1, typeofobj(args[i]));
    }
  }
  
  double x = __get_double((Number*)args[0]);
  double y = __get_double((Number*)args[1]);

  SDL_FPoint* point = malloc(sizeof(SDL_FPoint));

  if (!point) {
    return (Object*)initProgramError("Failed to create point object (Out of memory)");
  }

  *point = (SDL_FPoint){x, y};

  return (Object*)initInt((int64_t)(uintptr_t)point);
}

Object* arcUI_point_in_rect(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // point ptr
  if (err) return err;

  err = enforceType(args[1], OBJ_NUMBER_INT, 2); // rect ptr
  if (err) return err;
  
  SDL_FPoint *point = (SDL_FPoint*)(uintptr_t)((Number*)args[0])->as.i;
  SDL_FRect *rect = (SDL_FRect*)(uintptr_t)((Number*)args[1])->as.i;
  
  if (SDL_PointInRectFloat(point, rect)) {
    return (Object*)initInt(1);
  }

  return (Object*)initInt(0);
}
