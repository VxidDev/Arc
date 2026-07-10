import "__c_tools"
import "__lib_tools"
import "__sys"
import "__time"

var __mixer__ = dl_open(stdlib_path() + "/clib/libarcmixer.so", 1)
var mixer_init = dl_sym(__mixer__, "arcMixer_init", 0, false)
var mixer_load_sound = dl_sym(__mixer__, "arcMixer_load_audio", 2, false)
var mixer_quit = dl_sym(__mixer__, "arcMixer_quit", 0, false)
var mixer_play_sound = dl_sym(__mixer__, "arcMixer_play_sound", 3, false)
var mixer_destroy_mixer = dl_sym(__mixer__, "arcMixer_destroy_mixer", 1, false)
var mixer_track_playing = dl_sym(__mixer__, "arcMixer_track_playing", 1, false)
var mixer_disable_signal_handlers = dl_sym(__mixer__, "arcMixer_disable_signal_handlers", 0, false)

mixer_disable_signal_handlers()
var mixer = mixer_init()

var sound = mixer_load_sound(mixer, getenv("HOME") + "/Music/test.mp3")
mixer_play_sound(sound[1], sound[0], 0)

sleep(5000)

mixer_destroy_mixer(mixer)
mixer_quit()
