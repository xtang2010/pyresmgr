import json
import ctypes
from enum import Enum
from iofunc.iofunc import iofunc, iofunc_ocb


class ResourceManagerHandler():

    def __init__(self, mode = 0, lib = None):
        self.libpyres = lib if lib is not None else ctypes.CDLL("libpyresmgr.so")
        self.iofunc = iofunc(lib = self.libpyres, mode = mode)
        #self.iofunc.iofunc_init(mode)

    def register_io_handler(self, handler_map = None, callback = None):

        if handler_map is None:
            obj = {}
            subclass = type(self)
            for name in dir(ResourceManagerHandler):
                if callable(getattr(ResourceManagerHandler, name)) and not name.startswith("__"):
                    method = getattr(ResourceManagerHandler, name)
                    submethod = getattr(subclass, name)
                    if method != submethod:
                        obj[name] = submethod
        else:
            obj = handler_map
                    
        self.iofunc.iofunc_register(self, obj, callback)
    
    def ocb_alloc(self):
        return iofunc_ocb()

    def ocb_free(self, ocb):
        del ocb
        return
    
    def msgread(self, ocb, bufsize, offset):
        chunk = bytearray(bufsize)
        self.libpyres.py_resmgr_msgget.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ssize_t, ctypes.c_ssize_t]
        self.libpyres.py_resmgr_msgget.restype  = ctypes.c_ssize_t
        ocb_c = ctypes.c_void_p(ocb.ocb_c)
        buf_c = ctypes.c_void_p(ctypes.addressof(ctypes.c_ubyte.from_buffer(chunk)))
        size = self.libpyres.py_resmgr_msgget(ocb_c, buf_c, bufsize, offset)
        return chunk[:size] if size > 0 else None

    def msgwrite(self, ocb, buffer, offset):
        self.libpyres.py_resmgr_msgwrite.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ssize_t, ctypes.c_ssize_t]
        self.libpyres.py_resmgr_msgwrite.restype  = ctypes.c_ssize_t
        ocb_c = ctypes.c_void_p(ocb.ocb_c)
        buf_c = ctypes.c_void_p(buffer)
        return self.libpyres.py_resmgr_msgwrite(ocb_c, buf_c, len(buffer), offset)

    def _not_implemented(self):
        print("Not implemented")
        return -89, None

    # Connect funcs handlers, ENOSYS = 89
    def do_open(self, **kwargs):
        return self._not_implemented()
    
    def do_unlink(self, **kwargs):
        return self._not_implemented()
    
    def do_rename(self, **kwargs):
        return self._not_implemented()
    
    def do_mknod(self, **kwargs):
        return self._not_implemented()
    
    def do_readlink(self, **kwargs):
        return self._not_implemented()
    
    def do_link(self, **kwargs):
        return self._not_implemented()

    def do_unblock(self, **kwargs):
        return self._not_implemented()
    
    def do_mount(self, **kwargs):
        return self._not_implemented()
    
    def do_combo(self, **kwargs):
        return self._not_implemented()
    
    # IO funcs handlers
    def do_read(self, ocb, **kwargs):
        return self._not_implemented()
    
    def do_write(self, ocb, **kwargs):
        return self._not_implemented()

    def do_close_ocb(self, ocb, **kwargs):
        return self._not_implemented()
    
    def do_stat(self, ocb, **kwargs):
        return self._not_implemented()

    def do_select(self, ocb, **kwargs):
        return self._not_implemented()

    def do_devctl(self, ocb, **kwargs):
        return self._not_implemented()

    def do_unblock(self, ocb, **kwargs):
        return self._not_implemented()
    
    def do_pathconf(self, ocb, **kwargs):
        return self._not_implemented()
    
    def do_lseek(self, ocb, **kwargs):
        return self._not_implemented()
    
    def do_chmod(self, ocb, **kwargs):
        return self._not_implemented()
    
    def do_chown(self, ocb, **kwargs):
        return self._not_implemented()

    def do_utime(self, ocb, **kwargs):
        return self._not_implemented()

    def do_openfd(self, ocb, **kwargs):
        return self._not_implemented()

    def do_fdinfo(self, ocb, **kwargs):
        return self._not_implemented()

    def do_lock(self, ocb, **kwargs):
        return self._not_implemented()

    def do_space(self, ocb, **kwargs):
        return self._not_implemented()

    def do_shutdown(self, ocb, **kwargs):
        return self._not_implemented()

    def do_mmap(self, ocb, **kwargs):
        return self._not_implemented()

    def do_msg(self, ocb, **kwargs):
        return self._not_implemented()

    def do_check_msg(self, ocb, **kwargs):
        return self._not_implemented()

    def do_dup(self, ocb, **kwargs):
        return self._not_implemented()

    def do_close_dup(self, ocb, **kwargs):
        return self._not_implemented()

    def do_lock_ocb(self, ocb, **kwargs):
        return self._not_implemented()

    def do_unlock_ocb(self, ocb, **kwargs):
        return self._not_implemented()

    def do_sync(self, ocb, **kwargs):
        return self._not_implemented()

    def do_power(self, ocb, **kwargs):
        return self._not_implemented()

    def do_acl(self, ocb, **kwargs):
        return self._not_implemented()

    def do_pause(self, ocb, **kwargs):
        return self._not_implemented()

    def do_unpause(self, ocb, **kwargs):
        return self._not_implemented()

"""     
    def do_read64(self, ocb, **kwargs):
        return self._not_implemented()

    def do_write64(self, ocb, **kwargs):
        return self._not_implemented()

    def do_notify64(self, ocb, **kwargs):
        return self._not_implemented()

    def do_utime64(self, ocb, **kwargs):
        return self._not_implemented()
 """

if __name__ == "__main__":
    pass
    
    