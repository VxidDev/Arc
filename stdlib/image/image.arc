import "__c_tools"
import "__lib_tools"

var __IMG__ = dl_open(stdlib_path() + "/clib/libarcimage.so", 1)

var __IMG_load_texture = dl_sym(__IMG__, "arcImg_load_texture", 2, false)
var __IMG_render_texture = dl_sym(__IMG__, "arcImg_render_texture", 3, false)
var __IMG_destroy_texture = dl_sym(__IMG__, "arcImg_destroy_texture", 1, false)
var __IMG_render_texture_rotated = dl_sym(__IMG__, "arcImg_render_texture_rotated", 5, false)
var __IMG_register_flip_constants = dl_sym(__IMG__, "__registerFlipConstants", 0, false)

__IMG_register_flip_constants()

class Image
  var __img_ptr = null
  var is_initialized = false 

  fn init(self, img_ptr) then
    if self.is_initialized then
      RuntimeError("Object is already initialized")
    end
    
    self.__img_ptr = img_ptr
    self.is_initialized = true
  end

  fn cleanup(self) then
    if not self.is_initialized then
      RuntimeError("Object is not initialized")
    end

    __IMG_destroy_texture(self.__img_ptr)
    self.is_initialized = false
  end

  fn render(self, renderer, rect) then
    if not self.is_initialized then
      RuntimeError("Object is not initialized")
    end

    if typeof(renderer) != "Renderer" then
      RuntimeError("Argument 'renderer' is expected to be object of class Renderer")
    end
 
    if typeof(rect) == "Rect" then
      return __IMG_render_texture(renderer.__renderer_ptr, self.__img_ptr, rect.__rect_ptr)
    end

    if rect == null then      
      return __IMG_render_texture(renderer.__renderer_ptr, self.__img_ptr, null)
    end

    RuntimeError("Argument 'rect' is expected to be object of class Rect or null")
  end

  fn render_rotated(self, renderer, rect, angle, way_to_flip) then
    if not self.is_initialized then
      RuntimeError("Object is not initialized")
    end
 
    if typeof(renderer) != "Renderer" then
      RuntimeError("Argument 'renderer' is expected to be object of class Renderer")
    end
    
    if typeof(rect) == "Rect" then
      return __IMG_render_texture_rotated(renderer.__renderer_ptr, self.__img_ptr, rect.__rect_ptr, angle, way_to_flip)
    end

    if rect == null then      
      return __IMG_render_texture_rotated(renderer.__renderer_ptr, self.__img_ptr, null, angle, way_to_flip)
    end

    RuntimeError("Argument 'rect' is expected to be object of class Rect or null")
  end
end

class _image
  fn load_texture(renderer, path) then
    if typeof(renderer) != "Renderer" then
      RuntimeError("Argument 'renderer' must be an object of class Renderer")
    end

    var raw_img = __IMG_load_texture(renderer.__renderer_ptr, path)

    var img = Image()
    img.init(img, raw_img)

    return img
  end
end

var image = _image()
