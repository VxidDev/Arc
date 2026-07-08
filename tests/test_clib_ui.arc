import "__c_tools"
import "__lib_tools"

import "@stdlib/math.arc"

var __ui__ = dl_open(stdlib_path() + "/clib/libarcui.so", 1)

var ui_create_frame = dl_sym(__ui__, "arcUI_create_frame", 3, false)
var ui_delay = dl_sym(__ui__, "arcUI_delay", 1, false)
var ui_render_present = dl_sym(__ui__, "arcUI_render_present", 1, false)
var ui_quit = dl_sym(__ui__, "arcUI_quit", 0, false)
var ui_set_render_draw_color = dl_sym(__ui__, "arcUI_set_render_draw_color", 5, false)
var ui_render_clear = dl_sym(__ui__, "arcUI_render_clear", 1, false)
var ui_pump_events = dl_sym(__ui__, "arcUI_pump_events", 0, false)
var ui_poll_event = dl_sym(__ui__, "arcUI_poll_event", 0, false)

var frame = ui_create_frame(500, 500, "Arc Window")
var RUNNING = true 

var time = 0.0

while RUNNING then
  while true then 
    ev = ui_poll_event()

    if ev == 0 then 
      break 
    end

    if int_at(ev) == 256 then 
      RUNNING = false
      break
    end 
  end

  time = time + 0.03

  var r = to_int((sin(time) + 1.0) * 127.5)
  var g = to_int((sin(time + 2.094) + 1.0) * 127.5)
  var b = to_int((sin(time + 4.188) + 1.0) * 127.5)
  
  ui_set_render_draw_color(frame[1], r, g, b, 255)
  ui_render_clear(frame[1])
  
  ui_render_present(frame[1])

  ui_delay(16)
end 
