#include "../../../include/object.h"
#include "../../../include/utils.h"
#include "../../../include/repl/repl.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

Object* arcImg_load_texture(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // renderer
  if (err) return err;

  err = enforceType(args[1], OBJ_STRING, 2); // path
  if (err) return err;
  
  SDL_Renderer* renderer = (SDL_Renderer*)(uintptr_t)((Number*)args[0])->as.i;
  char* path = ((String*)args[1])->value;

  SDL_Texture* texture = IMG_LoadTexture(renderer, path);

  if (!texture) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Could not load texture: %s", SDL_GetError());

    return (Object*)initProgramError(buf);
  }

  return (Object*)initInt((int64_t)(uintptr_t)texture);
}

Object* arcImg_render_texture(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // renderer
  if (err) return err;

  err = enforceType(args[1], OBJ_NUMBER_INT, 2); // texture
  if (err) return err;

  if (args[2]->type != OBJ_NULL && args[2]->type != OBJ_NUMBER_INT) { // position
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 3 to be object of type 'null' or 'int', received '%s'", typeofobj(args[2]));
    return (Object*)initProgramError(buf);
  }

  SDL_Renderer* renderer = (SDL_Renderer*)(uintptr_t)((Number*)args[0])->as.i;
  SDL_Texture* texture = (SDL_Texture*)(uintptr_t)((Number*)args[1])->as.i;
  SDL_FRect* rect = args[2]->type == OBJ_NULL ? NULL : (SDL_FRect*)(uintptr_t)((Number*)args[2])->as.i;
  
  if (!SDL_RenderTexture(renderer, texture, NULL, rect)) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Could not render texture: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  return (Object*)initInt(1);
}

Object* __registerFlipConstants(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;
  
  setTable(variables, internIdentifier("FLIP_NONE", 9), VAL_INT(SDL_FLIP_NONE));
  setTable(variables, internIdentifier("FLIP_VERTICAL", 13), VAL_INT(SDL_FLIP_VERTICAL));
  setTable(variables, internIdentifier("FLIP_HORIZONTAL", 15), VAL_INT(SDL_FLIP_HORIZONTAL));

  return (Object*)initInt(1);
}

Object* arcImg_render_texture_rotated(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // renderer
  if (err) return err;

  err = enforceType(args[1], OBJ_NUMBER_INT, 2); // texture
  if (err) return err;

  if (args[2]->type != OBJ_NULL && args[2]->type != OBJ_NUMBER_INT) { // position
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 3 to be object of type 'null' or 'int', received '%s'", typeofobj(args[2]));
    return (Object*)initProgramError(buf);
  }
  
  if (args[3]->type != OBJ_NUMBER_INT && args[3]->type != OBJ_NUMBER_FLOAT) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Expected argument 4 to be object of type 'int' or 'float', received '%s'", typeofobj(args[3]));
  }

  err = enforceType(args[4], OBJ_NUMBER_INT, 5); // flip mode
  if (err) return err;

  SDL_Renderer* renderer = (SDL_Renderer*)(uintptr_t)((Number*)args[0])->as.i;
  SDL_Texture* texture = (SDL_Texture*)(uintptr_t)((Number*)args[1])->as.i;
  SDL_FRect* rect = args[2]->type == OBJ_NULL ? NULL : (SDL_FRect*)(uintptr_t)((Number*)args[2])->as.i;
  int64_t angle = ((Number*)args[3])->as.i;
  SDL_FlipMode flip = (SDL_FlipMode)((Number*)args[4])->as.i;

  if (!SDL_RenderTextureRotated(renderer, texture, NULL, rect, (double)angle, NULL, flip)) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Could not render rotated texture: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  return (Object*)initInt(1);
}

Object* arcImg_destroy_texture(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // texture
  if (err) return err;

  SDL_Texture* texture = (SDL_Texture*)(uintptr_t)((Number*)args[0])->as.i;
  SDL_DestroyTexture(texture);

  return (Object*)initInt(1);
}
