import "__c_tools"
import "__lib_tools"
import "__sys"

if get_os() == "Linux" then
  var __mixer__ = dl_open(stdlib_path() + "/clib/libarcmixer.so")
elif get_os() == "MacOS" then
  var __mixer__ = dl_open(stdlib_path() + "/clib/libarcmixer.dylib")
elif get_os() == "Windows" then
  var __mixer__ = dl_open(stdlib_path() + "\\clib\\libarcmixer.dll")
else 
  RuntimeError("Unknown OS.")
end

var __mixer_init = dl_sym(__mixer__, "arcMixer_init", 0, false)
var __mixer_load_sound = dl_sym(__mixer__, "arcMixer_load_audio", 2, false)
var __mixer_quit = dl_sym(__mixer__, "arcMixer_quit", 0, false)
var __mixer_play_sound = dl_sym(__mixer__, "arcMixer_play_sound", 3, false)
var __mixer_destroy_mixer = dl_sym(__mixer__, "arcMixer_destroy_mixer", 1, false)
var __mixer_track_playing = dl_sym(__mixer__, "arcMixer_track_playing", 1, false)
var __mixer_disable_signal_handlers = dl_sym(__mixer__, "arcMixer_disable_signal_handlers", 0, false)

class Sound
  var __track_ptr = null
  var __audio_ptr = null

  var is_initialized = false

  fn init(self, track_ptr, audio_ptr) then    
    if self.is_initialized then
      RuntimeError("Object is already initialized")
    end

    self.__track_ptr = track_ptr
    self.__audio_ptr = audio_ptr
    self.is_initialized = true
  end
end

class Mixer
  var __mixer_ptr = null
  var is_initialized = false

  fn init(self) then
    if self.is_initialized then
      RuntimeError("Object is already initialized")
    end

    self.__mixer_ptr = __mixer_init()
    self.is_initialized = true
  end

  fn load_sound(self, path) then
    if not self.is_initialized then
      RuntimeError("Object is not initialized")
    end

    var sound_raw = __mixer_load_sound(self.__mixer_ptr, path)

    var sound = Sound()
    sound.init(sound, sound_raw[1], sound_raw[0])

    return sound
  end

  fn play_sound(self, sound, loop) then
    if not self.is_initialized then
      RuntimeError("Object is not initialized")
    end

    __mixer_play_sound(sound.__track_ptr, sound.__audio_ptr, loop)
  end

  fn cleanup(self) then
    __mixer_destroy_mixer(self.__mixer_ptr)
  end
end

class _mixer
  fn disable_signal_handlers() then
    __mixer_disable_signal_handlers()
  end

  fn create_mixer() then
    var mixer = Mixer()
    mixer.init(mixer)

    return mixer
  end

  fn quit() then
    __mixer_quit()
  end
end

mixer = _mixer()
