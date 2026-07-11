import "__c_tools"
import "__lib_tools"

import "@stdlib/libc/stdlib.arc"

var __ui__ = dl_open(stdlib_path() + "/clib/libarcui.so", 1)

var __UI_register_sdl_keys = dl_sym(__ui__, "__registerSDLKeys", 0, false)
var __UI_create_stage = dl_sym(__ui__, "arcUI_create_stage", 3, false)
var __UI_delay = dl_sym(__ui__, "arcUI_delay", 1, false)
var __UI_render_present = dl_sym(__ui__, "arcUI_render_present", 1, false)
var __UI_quit = dl_sym(__ui__, "arcUI_quit", 0, false)
var __UI_set_render_draw_color = dl_sym(__ui__, "arcUI_set_render_draw_color", 5, false)
var __UI_render_clear = dl_sym(__ui__, "arcUI_render_clear", 1, false)
var __UI_pump_events = dl_sym(__ui__, "arcUI_pump_events", 0, false)
var __UI_poll_event = dl_sym(__ui__, "arcUI_poll_event", 0, false)
var __UI_rect = dl_sym(__ui__, "arcUI_rect", 4, false)
var __UI_fill_rect = dl_sym(__ui__, "arcUI_fill_rect", 2, false)
var __UI_get_mouse_info = dl_sym(__ui__, "arcUI_get_mouse_info", 1, false)
var __UI_point = dl_sym(__ui__, "arcUI_point", 2, false)
var __UI_point_in_rect = dl_sym(__ui__, "arcUI_point_in_rect", 2, false)
var __UI_get_event_type = dl_sym(__ui__, "arcUI_get_event_type", 1, false)
var __UI_get_window_size_pixels = dl_sym(__ui__, "arcUI_get_window_size_pixels", 1, false)
var __UI_set_window_resizable = dl_sym(__ui__, "arcUI_set_window_resizable", 2, false)
var __UI_update_rect_pos = dl_sym(__ui__, "arcUI_update_rect_pos", 3, false)
var __UI_get_key_info = dl_sym(__ui__, "arcUI_get_key_info", 1, false)

class Rect
  var __rect_ptr = null
  var x = null
  var y = null

  var left = null
  var right = null
  var bottom = null
  var top = null
  
  var width = null
  var height = null

  var thickness = 5.0

  var is_initialized = false

  fn init(self, rect_ptr, x, y, width, height, has_border) then
    if self.is_initialized then
      RuntimeError("Object is already initialized")
    end

    self.__rect_ptr = rect_ptr

    self.x = x
    self.y = y

    self.width = width
    self.height = height

    self.has_border = has_border
    
    if has_border then
      var left_rect_x = x - self.thickness
      var left_rect_y = y
      var left_rect_w = self.thickness
      var left_rect_h = height
      var left_rect = Rect()
      self.left = left_rect.init(left_rect, __UI_rect(left_rect_x, left_rect_y, left_rect_w, left_rect_h), left_rect_x, left_rect_y, left_rect_w, left_rect_h, false)

      var right_rect_x = x + width
      var right_rect_y = y
      var right_rect_w = self.thickness
      var right_rect_h = height
      var right_rect = Rect()
      self.right = right_rect.init(right_rect, __UI_rect(right_rect_x, right_rect_y, right_rect_w, right_rect_h), right_rect_x, right_rect_y, right_rect_w, right_rect_h, false)

      var top_rect_x = x
      var top_rect_y = y - self.thickness
      var top_rect_w = width
      var top_rect_h = self.thickness
      var top_rect = Rect()
      self.top = top_rect.init(top_rect, __UI_rect(top_rect_x, top_rect_y, top_rect_w, top_rect_h), top_rect_x, top_rect_y, top_rect_w, top_rect_h, false)

      var bottom_rect_x = x
      var bottom_rect_y = y + height
      var bottom_rect_w = width
      var bottom_rect_h = self.thickness
      var bottom_rect = Rect()
      self.bottom = bottom_rect.init(bottom_rect, __UI_rect(bottom_rect_x, bottom_rect_y, bottom_rect_w, bottom_rect_h), bottom_rect_x, bottom_rect_y, bottom_rect_w, bottom_rect_h, false)
    end

    self.is_initialized = true
    return self
  end

  fn set_pos(self, x, y) then
    if not self.is_initialized then
      RuntimeError("Object is not initialized")
    end

    __UI_update_rect_pos(self.__rect_ptr, x, y)

    self.x = x
    self.y = y

    __UI_update_rect_pos(self.left.__rect_ptr, self.x - self.thickness, self.y)
    
    self.left.x = self.x - self.thickness
    self.left.y = self.y

    __UI_update_rect_pos(self.right.__rect_ptr, self.x + self.width, self.y)

    self.right.x = self.x + self.width
    self.right.y = self.y

    __UI_update_rect_pos(self.top.__rect_ptr, self.x, self.y - self.thickness)

    self.top.x = self.x
    self.top.y = self.y - self.thickness

    __UI_update_rect_pos(self.bottom.__rect_ptr, self.x, self.y + self.height)

    self.bottom.x = self.x
    self.bottom.y = self.y + self.height
  end

  fn collides_with(self, rect) then
    if not self.is_initialized then
      RuntimeError("Object is not initialized")
    end

    var point_ptr = __UI_point(rect.x, rect.y)
    return __UI_point_in_rect(point_ptr, self.__rect_ptr)
  end
end

class Window
  var width = null
  var height = null
  var name = null
  
  var __window_ptr = null

  var is_initialized = false

  fn init(self, width, height, name, window_ptr) then
    if self.is_initialized then 
      RuntimeError("Object is already initialized")
    end

    if typeof(width) != "int" then 
      RuntimeError("Argument 'width' must be an integer")
    end 

    if typeof(height) != "int" then 
      RuntimeError("Argument 'height' must be an integer")
    end

    if typeof(name) != "string" then 
      RuntimeError("Argument 'name' must be a string")
    end 

    if typeof(window_ptr) != "int" then 
      RuntimeError("Argument 'window_ptr' must be an integer")
    end 

    self.width = width
    self.height = height
    self.name = name
    self.__window_ptr = window_ptr

    self.is_initialized = true
  end
end 

class Renderer
  var __renderer_ptr = null
  var is_initialized = false

  fn init(self, renderer_ptr) then
    if self.is_initialized then 
      RuntimeError("Object is already initialized")
    end

    if typeof(renderer_ptr) != "int" then 
      RuntimeError("Argument 'renderer_ptr' must be an integer")
    end 

    self.__renderer_ptr = renderer_ptr
    self.is_initialized = true
  end
end 

class Stage
  window = null
  renderer = null
  
  is_initialized = false

  fn init(self, window, renderer) then 
    if self.is_initialized then 
      RuntimeError("Object is already initialized")
    end 
    
    if typeof(window) != "Window" then
      RuntimeError("Argument 'window' must be an instance of class Window")
    end

    if typeof(renderer) != "Renderer" then
      RuntimeError("Argument 'renderer' must be an instance of class Renderer") 
    end

    self.window = window
    self.renderer = renderer

    self.is_initialized = true
  end

  fn clear(self) then
    __UI_render_clear(self.renderer.__renderer_ptr)
  end

  fn present(self) then
    __UI_render_present(self.renderer.__renderer_ptr)
  end

  fn fill(self, color) then
    if typeof(color) != "list" then
      RuntimeError("Argument 'color' must be a list of integers")
    end

    if len_of(color) != 4 then
      RuntimeError("Argument 'color' must be a list of length 4")
    end

    __UI_set_render_draw_color(self.renderer.__renderer_ptr, color[0], color[1], color[2], color[3])
    __UI_render_clear(self.renderer.__renderer_ptr)
  end

  fn draw_rect(self, rect, color) then
    if typeof(color) != "list" then
      RuntimeError("Argument 'color' must be a list of integers")
    end

    if len_of(color) != 4 then
      RuntimeError("Argument 'color' must be a list of length 4")
    end

    if typeof(rect) != "Rect" then
      RuntimeError("Argument 'rect' must be an instance of class Rect")
    end

    __UI_set_render_draw_color(self.renderer.__renderer_ptr, color[0], color[1], color[2], color[3])
    __UI_fill_rect(self.renderer.__renderer_ptr, rect.__rect_ptr)
    __UI_set_render_draw_color(self.renderer.__renderer_ptr, 0, 0, 0, 0)
  end

  fn get_window_size_in_pixels(self) then
    __UI_get_window_size_in_pixels(self.window.__window_ptr)
  end

  fn set_window_resizable(self, is_resizable) then
    if (is_resizable != 0) and (is_resizable != 1) then
      RuntimeError("Argument 'is_resizable' must be an integer of value 1 or 0")
    end

    __UI_set_window_resizable(self.window.__window_ptr, is_resizable)
  end
end 

class Event
  var type = null 
  var __event_ptr = null
  var is_initialized = false

  fn init(self, ev) then
    if self.is_initialized then
      RuntimeError("Object is already initialized")
    end

    if typeof(ev) != "int" then
      RuntimeError("Argument 'ev' must be an integer")
    end

    self.type = __UI_get_event_type(ev)
    self.__event_ptr = ev

    self.is_initialized = true
  end

  fn cleanup(self) then
    if not self.is_initialized then
      RuntimeError("Object is not initialized")
    end

    free(self.__event_ptr)
  end
end

class _ui
  var MOUSE_BUTTON_DOWN = 0
  var MOUSE_BUTTON_UP = 1
  var MOUSE_WHEEL = 2
  var KEY_DOWN = 3
  var KEY_UP = 4
  var QUIT = 5
  var NOT_MAPPED = 6

  fn create_stage(width, height, window_name) then 
    var __stage_raw = __UI_create_stage(width, height, window_name) # enforces types
    
    var window = Window()
    window.init(window, width, height, window_name, __stage_raw[0])
    
    var renderer = Renderer()
    renderer.init(renderer, __stage_raw[1])

    var stage = Stage()
    stage.init(stage, window, renderer)

    return stage
  end

  fn get_event() then
    var raw_ev = __UI_poll_event()

    if raw_ev == 0 then
      return null
    end

    var event = Event()
    event.init(event, raw_ev)

    return event
  end

  fn rect(x, y, width, height) then
    var rect_raw = __UI_rect(x, y, width, height) # enforces types

    var rect = Rect()
    rect.init(rect, rect_raw, x, y, width, height, true)

    return rect
  end

  fn delay(ms) then
    __UI_delay(ms)
  end

  fn get_mouse_info(ev) then
    if typeof(ev) != "Event" then
      RuntimeError("Argument 'ev' must be an instance of class Event")
    end

    __UI_get_mouse_info(ev.__event_ptr)
  end

  fn get_key_info(ev) then
    if typeof(ev) != "Event" then
      RuntimeError("Argument 'ev' must be an instance of class Event")
    end

    __UI_get_key_info(ev.__event_ptr)
  end
end

__UI_register_sdl_keys()
var ui = _ui() 
