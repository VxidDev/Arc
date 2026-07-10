import "__c_tools"
import "__lib_tools"

var __ui__ = dl_open(stdlib_path() + "/clib/libarcui.so", 1)

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

class Window
  var width = null
  var height = null
  var name = null
  
  var __window_ptr = null

  var is_initialized = false

  fn init(self, width, height, name, window_ptr) then
    if self.is_initialized then 
      RuntimeError("Object is already initialized.")
    end

    if typeof(width) != "int" then 
      RuntimeError("Argument 'width' must be an integer.")
    end 

    if typeof(height) != "int" then 
      RuntimeError("Argument 'height' must be an integer.")
    end

    if typeof(name) != "string" then 
      RuntimeError("Argument 'name' must be a string.")
    end 

    if typeof(window_ptr) != "int" then 
      RuntimeError("Argument 'window_ptr' must be an integer.")
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
      RuntimeError("Object is already initialized.")
    end

    if typeof(renderer_ptr) != "int" then 
      RuntimeError("Argument 'renderer_ptr' must be an integer.")
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
end 

class Event
  var type = null 

  var is_initialized = false

  fn init(self, ev) then
    if self.is_initialized then
      RuntimeError("Object is already initialized")
    end

    if typeof(ev) != "int" then
      RuntimeError("Argument 'ev' must be an integer.")
    end

    self.type = __UI_get_event_type(ev)
    self.is_initialized = true
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
end

var ui = _ui() 
