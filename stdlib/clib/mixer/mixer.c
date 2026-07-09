#include "../../../include/object.h"
#include "../../../include/utils.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

Object* arcMixer_init(Object** args, size_t argCount) {
  (void)argCount;
  (void)args;

  if (!SDL_Init(SDL_INIT_AUDIO)) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Could not initialize SDL's audio-system: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  if (!MIX_Init()) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Could not initialize SDL_mixer: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }
  
  MIX_Mixer* mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);

  if (!mixer) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Could not create mixer: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  return (Object*)initInt((int64_t)(uintptr_t)mixer);
}

Object* arcMixer_quit(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;

  MIX_Quit();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);

  return (Object*)initInt(1);
}

Object* arcMixer_load_audio(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // mixer
  if (err) return err;

  err = enforceType(args[1], OBJ_STRING, 2); // sound path
  if (err) return err;

  MIX_Mixer* mixer = (MIX_Mixer*)(uintptr_t)((Number*)args[0])->as.i;
  char* path = ((String*)args[1])->value;

  MIX_Audio* sound = MIX_LoadAudio(mixer, path, true);

  if (!sound) {
    char buf[512];
    snprintf(buf, sizeof(buf), "Could not load sound: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  MIX_Track* track = MIX_CreateTrack(mixer);

  if (!track) {
    MIX_DestroyAudio(sound);

    char buf[512];
    snprintf(buf, sizeof(buf), "Could not create track: %s", SDL_GetError());
    return (Object*)initProgramError(buf);
  }

  Object* sd = (Object*)initInt((int64_t)(uintptr_t)sound);
  Object* tk = (Object*)initInt((int64_t)(uintptr_t)track);

  return (Object*)initList((Object*[]){ sd, tk }, 2, 2);
}

Object* arcMixer_play_sound(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // track
  if (err) return err;

  err = enforceType(args[1], OBJ_NUMBER_INT, 2); // sound
  if (err) return err;

  err = enforceType(args[2], OBJ_NUMBER_INT, 3); // loop
  if (err) return err;

  MIX_Track* track = (MIX_Track*)(uintptr_t)((Number*)args[0])->as.i;
  MIX_Audio* audio = (MIX_Audio*)(uintptr_t)((Number*)args[1])->as.i;
  int64_t loop = ((Number*)args[2])->as.i; // -1 = loop forever / 0 = play once

  MIX_SetTrackAudio(track, audio);
  MIX_PlayTrack(track, loop);
  
  return (Object*)initInt(1);
}

Object* arcMixer_destroy_mixer(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1); // mixer
  if (err) return err;

  MIX_Mixer* mixer = (MIX_Mixer*)(uintptr_t)((Number*)args[0])->as.i;

  MIX_DestroyMixer(mixer);

  return (Object*)initInt(1);
}

Object* arcMixer_track_playing(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  MIX_Track* track = (MIX_Track*)(uintptr_t)((Number*)args[0])->as.i;

  if (MIX_TrackPlaying(track)) {
    return (Object*)initInt(1);
  } else {
    return (Object*)initInt(0);
  }
}
