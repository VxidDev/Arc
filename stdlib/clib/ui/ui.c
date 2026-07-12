#include "../../../include/object.h"
#include "../../../include/utils.h"
#include "../../../include/repl/repl.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <SDL3/SDL.h>

static SDL_Event globalEv;

// create_stage(width: int, height: int, name: string) -> int64_t
Object* arcUI_create_stage(Object** args, size_t argCount) {
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
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
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

  SDL_SyncWindow(window);
  
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

typedef enum ARCUI_EVENT {
  MOUSE_BUTTON_DOWN,
  MOUSE_BUTTON_UP,
  MOUSE_WHEEL,
  KEY_DOWN,
  KEY_UP,
  QUIT,
  NOT_MAPPED
} ARCUI_EVENT;

Object* __getEventType(SDL_Event *ev) { 
  uint32_t type;

  switch (ev->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: type = MOUSE_BUTTON_DOWN; break;
    case SDL_EVENT_MOUSE_BUTTON_UP: type = MOUSE_BUTTON_UP; break;
    case SDL_EVENT_MOUSE_WHEEL: type = MOUSE_WHEEL; break;
    case SDL_EVENT_KEY_DOWN: type = KEY_DOWN; break;
    case SDL_EVENT_KEY_UP: type = KEY_UP; break;
    case SDL_EVENT_QUIT: type = QUIT; break;

    default: type = NOT_MAPPED; break;
  }

  return (Object*)initInt(type);
}

bool __appendList(Object** pairs, char *key, Object* value, size_t *idx) {
  if (!value) {
    for (size_t i = 0; i < *idx; i++) {
      freeObject(pairs[i]);
    }

    free(pairs);

    return false;
  }

  Object* keyString = (Object*)initString(key, strlen(key));
  
  if (!keyString) {
    for (size_t i = 0; i < *idx; i++) {
      freeObject(pairs[i]);
    }

    free(pairs);
    freeObject(value);

    return false;
  }

  Object* pair = (Object*)initList((Object*[]){keyString, value}, 2, 2);

  if (!pair) {
    for (size_t i = 0; i < *idx; i++) {
      freeObject(pairs[i]);
    }

    free(pairs);

    freeObject(keyString);
    freeObject(value);

    return false;
  }

  pairs[*idx] = pair;
  (*idx)++;

  return true;
}

Object* __arcUIEventToList(SDL_Event* ev) {
  size_t idx = 0;
  Object** pairs = malloc(sizeof(Object*) * 8);
  
  if (!pairs) {
    return (Object*)initProgramError("Failed to parse event (Out of memory)");
  }

  if (!__appendList(pairs, "type", (Object*)__getEventType(ev), &idx))
    return (Object*)initProgramError("Failed to parse event (Out of memory)");

  switch (ev->type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
      const char* keyname = SDL_GetKeyName(ev->key.key);

      if (!__appendList(pairs, "key", (Object*)initString(keyname, strlen(keyname)), &idx))
        return (Object*)initProgramError("Failed to parse event (Out of memory)");

      if (!__appendList(pairs, "down", (Object*)initInt(ev->type == SDL_EVENT_KEY_DOWN), &idx))
        return (Object*)initProgramError("Failed to parse event (Out of memory)");

      if (!__appendList(pairs, "scancode", (Object*)initInt(ev->key.scancode), &idx))
        return (Object*)initProgramError("Failed to parse event (Out of memory)");

      break;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (!__appendList(pairs, "mouse_pos_x", (Object*)initFloat(ev->button.x), &idx))
        return (Object*)initProgramError("Failed to parse event (Out of memory)");

      if (!__appendList(pairs, "mouse_pos_y", (Object*)initFloat(ev->button.y), &idx))
        return (Object*)initProgramError("Failed to parse event (Out of memory)");

      if (!__appendList(pairs, "clicked_button", (Object*)initInt(ev->button.button == SDL_BUTTON_LEFT ? 0 : 1), &idx))
        return (Object*)initProgramError("Failed to parse event (Out of memory)");
      
      if (!__appendList(pairs, "down", (Object*)initInt(ev->type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1 : 0), &idx))
        return (Object*)initProgramError("Failed to parse event (Out of memory)");

      break;
    }

    case SDL_EVENT_QUIT: {
      break;
    }

    default:
      break;
  }

  Object* list = (Object*)initList(pairs, idx, idx);
  free(pairs);

  return list;
}

Object* arcUI_poll_event(Object** args, size_t argCount) {
  (void)argCount;
  (void)args;

  if (SDL_PollEvent(&globalEv)) {
    return __arcUIEventToList(&globalEv);
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

Object* arcUI_update_rect_pos(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // rect ptr
  if (err) return err;

  for (size_t i = 1; i < 3; i++) {
    if (args[i]->type != OBJ_NUMBER_INT && args[i]->type != OBJ_NUMBER_FLOAT) {
      char buf[256];
      snprintf(buf, sizeof(buf), "Expected argument %zu to be object of type 'int' or 'float', received '%s'", i + 1, typeofobj(args[i]));
    }
  }
  
  SDL_FRect *rect = (SDL_FRect*)(uintptr_t)((Number*)args[0])->as.i;

  double x = __get_double((Number*)args[1]);
  double y = __get_double((Number*)args[2]);
  
  rect->x = x;
  rect->y = y;

  return (Object*)initInt(0);
}

Object* __registerSDLKeys(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;

  for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
    // Create a unique name for each key, e.g., "KEY_A", "KEY_W"
    const char* name = SDL_GetScancodeName((SDL_Scancode)i);
        
    if (name && name[0] != '\0') {
      char fullname[256];
      
      snprintf(fullname, sizeof(fullname), "KEY_%s", name);

      for (int j = 4; fullname[j] != '\0'; j++) {
        fullname[j] = toupper((unsigned char)fullname[j]);
      }

      setTable(variables, internIdentifier(fullname, strlen(fullname)), VAL_OBJ((Object*)initInt(i)));
    }
  }

  return (Object*)initInt(1);
}

Object* arcUI_get_window_size_pixels(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // window
  if (err) return err;

  SDL_Window* window = (SDL_Window*)(uintptr_t)((Number*)args[0])->as.i;

  int w, h;

  if (!SDL_GetWindowSizeInPixels(window, &w, &h)) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Could not get window size: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }
  
  Object* width = (Object*)initInt(w);
  Object* height = (Object*)initInt(h);

  return (Object*)initList((Object*[]){width, height}, 2, 2);
}

Object* arcUI_set_window_resizable(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // window
  if (err) return err;

  err = enforceType(args[1], OBJ_NUMBER_INT, 2); // is resizable
  if (err) return err;

  SDL_Window* window = (SDL_Window*)(uintptr_t)((Number*)args[0])->as.i;
  int64_t isResizable = ((Number*)args[1])->as.i;

  if (!SDL_SetWindowResizable(window, isResizable)) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Could not update window resizability: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }
  
  return (Object*)initInt(1);
}

Object* arcUI_destroy_window(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // window 
  if (err) return err;
  
  SDL_Window* window = (SDL_Window*)(uintptr_t)((Number*)args[0])->as.i;
  SDL_DestroyWindow(window);

  return (Object*)initInt(1);
}
