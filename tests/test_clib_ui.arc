import "__c_tools"
import "__lib_tools"

import "@stdlib/math.arc"

var __ui__ = dl_open(stdlib_path() + "/clib/libarcui.so", 1)

var ui_create_stage = dl_sym(__ui__, "arcUI_create_stage", 3, false)
var ui_delay = dl_sym(__ui__, "arcUI_delay", 1, false)
var ui_render_present = dl_sym(__ui__, "arcUI_render_present", 1, false)
var ui_quit = dl_sym(__ui__, "arcUI_quit", 0, false)
var ui_set_render_draw_color = dl_sym(__ui__, "arcUI_set_render_draw_color", 5, false)
var ui_render_clear = dl_sym(__ui__, "arcUI_render_clear", 1, false)
var ui_pump_events = dl_sym(__ui__, "arcUI_pump_events", 0, false)
var ui_poll_event = dl_sym(__ui__, "arcUI_poll_event", 0, false)
var ui_rect = dl_sym(__ui__, "arcUI_rect", 4, false)
var ui_fill_rect = dl_sym(__ui__, "arcUI_fill_rect", 2, false)
var ui_get_mouse_info = dl_sym(__ui__, "arcUI_get_mouse_info", 1, false)
var ui_point = dl_sym(__ui__, "arcUI_point", 2, false)
var ui_point_in_rect = dl_sym(__ui__, "arcUI_point_in_rect", 2, false)

var stage = ui_create_stage(500, 500, "Arc Window")
var RUNNING = true 

var button_rect = ui_rect(200, 200, 50, 50)

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

    if (int_at(ev) == 1026) or (int_at(ev) == 1025) then 
      var info = ui_get_mouse_info(ev)

      var point = ui_point(info[0], info[1])

      print("Mouse button:", if info[2] == 0 then "left" else "right" end, "Status:", if info[3] == 1 then "down" else "up" end, "X:", info[0], "Y:", info[1], "Collides rect:", ui_point_in_rect(point, button_rect))
    end 
  end

  time = time + 0.03

  var r = to_int((sin(time) + 1.0) * 127.5)
  var g = to_int((sin(time + 2.094) + 1.0) * 127.5)
  var b = to_int((sin(time + 4.188) + 1.0) * 127.5)
  
  ui_set_render_draw_color(stage[1], r, g, b, 255)
  ui_render_clear(stage[1])

  ui_set_render_draw_color(stage[1], 30, 30, 30, 255)
  ui_fill_rect(stage[1], button_rect)
  
  ui_render_present(stage[1])

  ui_delay(16)
end 
