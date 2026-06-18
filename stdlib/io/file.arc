FILE_SEEK_SET = 0
FILE_SEEK_CUR = 1
FILE_SEEK_END = 2

class File 
  var filename = ""
  var mode = ""
  var _file_obj = -1

  var is_open = false
  
  fn is_initialized(self) then 
    if self.mode == "" or self.filename == "" then 
      return false 
    end 

    return true 
  end 

  fn init(self, filename, mode) then 
    self.filename = filename
    self.mode = mode 

    return self
  end 

  fn open(self) then
    if self.is_open then 
      RuntimeError("File is already opened.")
    end

    if not self.is_initialized(self) then 
      RuntimeError("File is not initialized.")
    end 
    
    self._file_obj = open_file(self.filename, self.mode)
    self.is_open = true

    return self
  end 

  fn close(self) then 
    if not self.is_open then 
      RuntimeError("File is already closed.")
    end 

    close_file(self._file_obj)

    self.is_open = false
    self._file_obj = -1

    return self
  end

  fn write(self, string) then
    if not self.is_initialized(self) then 
      RuntimeError("File is not initialized.")
    end 

    if not self.is_open then 
      RuntimeError("File is not opened.")
    end 

    write_file(self._file_obj, string)

    return self
  end 

  fn read(self) then 
    if not self.is_initialized(self) then 
      RuntimeError("File is not initialized.")
    end 

    if not self.is_open then 
      RuntimeError("File is not opened.")
    end

    return read_file(self._file_obj)
  end

  fn read_char(self) then
    if not self.is_initialized(self) then
      RuntimeError("File is not initialized.")
    end 

    if not self.is_open then 
      RuntimeError("File is not opened.")
    end 

    return stream_read_char(self._file_obj)
  end

  fn seek(self, offset, whence) then
    if not self.is_initialized(self) then
      RuntimeError("File is not initialized")
    end 

    if not self.is_open then
      RuntimeError("File is not opened.")
    end

    return stream_seek(self._file_obj, offset, whence) 
  end

  fn tell(self) then
    if not self.is_initialized(self) then
      RuntimeError("File is not initialized")
    end 

    if not self.is_open then
      RuntimeError("File is not opened.")
    end 

    return stream_tell(self._file_obj) 
  end

  fn get_size(self) then 
    var curr_pos = self.tell(self)

    self.seek(self, 0, FILE_SEEK_END)
    var size = self.tell(self)
    self.seek(self, curr_pos, FILE_SEEK_SET)

    return size
  end 
end 
