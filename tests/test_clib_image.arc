import "@stdlib/ui/ui.arc"

import "__c_tools"
import "__lib_tools"
import "__sys"

if get_os() == "Linux" then
  var __img__ = dl_open(stdlib_path() + "/clib/libarcimage.so")
elif get_os() == "MacOS" then
  var __img__ = dl_open(stdlib_path() + "/clib/libarcimage.dylib") 
elif get_os() == "Windows" then
  var __img__ = dl_open(stdlib_path() + "\\clib\\libarcimage.dll")
else
  RuntimeError("Unknown OS.")
end

var img_load_texture = dl_sym(__img__, "arcImg_load_texture", 2, false)
var img_render_texture = dl_sym(__img__, "arcImg_render_texture", 3, false)

var stage = ui.create_stage(400, 400, "clib image test")
stage.set_window_resizable(stage, false)

var RUNNING = true

var image = img_load_texture(stage.renderer.__renderer_ptr, getenv("HOME") + "/Pictures/test.png")
var rect = ui.rect(0, 0, 400, 400)

while RUNNING then
  while true then
    var event = ui.get_event()

    if event == null then
      break
    end

    if event.type == ui.QUIT then
      RUNNING = false
    end
  end
  
  img_render_texture(stage.renderer.__renderer_ptr, image, rect.__rect_ptr)

  stage.present(stage)
  ui.delay(16)
end
