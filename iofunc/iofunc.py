import ctypes
import sys
from enum import Enum

class iofunc_connect_handler(Enum):
    OPEN       = 2
    UNLINK     = 3
    RENAME     = 4
    MKNOD      = 6
    RADLINK    = 7
    LINK       = 8
    UNBLOCK    = 9
    MOUNT      = 10
    COMBO      = 11

class iofunc_io_handler(Enum):
    READ       = 0x101
    WRITE      = 0x102
    CLOSE_OCB  = 0x103
    STAT       = 0x104
    NOTIFY     = 0x105
    DEVCTL     = 0x106
    UNBLOCK    = 0x107
    PATHCONF   = 0x108
    LSEEK      = 0x109
    CHMOD      = 0x10A
    CHOWN      = 0x10B
    UTIME      = 0x10C
    OPENFD     = 0x10D
    FDINFO     = 0x10E
    LOCK       = 0x10F
    SPACE      = 0x110
    SHUTDOWN   = 0x111
    MMAP       = 0x112
    MSG        = 0x113
    CHECK_MSG  = 0x114
    DUP        = 0x115
    CLOSE_DUP  = 0x116
    LOCK_OCB   = 0x117
    UNLOCK_OCB = 0x118
    SYNC       = 0x119
    POWER      = 0x11A
    ACL        = 0x11B
    PAUSE      = 0x11C
    UNPAUSE    = 0x11D
    READ64     = 0x11E
    WRITE64    = 0x11F
    NOTIFY64   = 0x120
    UTIME64    = 0x121

class RegisterHandle(ctypes.Structure):
    _fields_ = [
        ("id",   ctypes.c_int),
        ("func", ctypes.py_object)
    ]

class iofunc_ocb():

    def __init__(self, lib = None):
        self.ocb_c    = None
        self.offset   = 0
        self.libpyres = ctypes.CDLL("libpyresmgr.so") if lib is None else lib
        # self.libpyres.pyres_ocb_get.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        # self.libpyres.pyres_ocb_get.restype  = ctypes.c_int64
        # self.libpyres.pyres_ocb_set.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int64]
        # self.libpyres.pyres_ocb_set.restype  = ctypes.c_int
        
    # def __getattr__(self, item):
    #     if item != "offset" and self.ocb_c is not None:
    #         return self.libpyres.pyres_ocb_get(self.ocb_c, ctypes.c_char_p(item.encode('utf-8')))
    #     return self.offset

    # def __setattr__(self, item, value):
    #     if item == "offset" and self.ocb_c is not None:
    #         self.libpyres.pyres_ocb_set(self.ocb_c, ctypes.c_char_p(item.encode('utf-8')), value)
    #     super.__setattr__(self, item, value)

class iofunc():
    def __init__(self, lib = None, mode = 0):
        self.libpyres = ctypes.CDLL("libpyresmgr.so") if lib is None else lib
        self.libpyres.py_iofunc_block_init.argtypes = [ctypes.c_int]
        self.libpyres.py_iofunc_block_init.restype  = ctypes.c_void_p
        self.iofunc_block_c = self.libpyres.py_iofunc_block_init(mode)

    def iofunc_register(self, handler_instance, fmap, callback=None):

        self.instance = handler_instance
        self.register_handler = {}

        instance = handler_instance
        method_name = ctypes.c_char_p(callback.encode('utf-8')) if callback is not None else ctypes.c_char_p(b"iofunc_callback")

        register_array = (RegisterHandle * len(fmap))()
        i = 0
        for k, v in fmap.items():
            if k.startswith("do_") is not True:
                continue
            k = k[3:].upper()
            if (k in iofunc_connect_handler.__members__):
                id = iofunc_connect_handler.__members__[k].value
            elif (k in iofunc_io_handler.__members__):
                id = iofunc_io_handler.__members__[k].value
            else:
                raise(Exception(f"Unknow method {fmap.items}"))
            
            self.register_handler[str(id)] = v
            register_array[i].id = id
            register_array[i].func = ctypes.py_object(v)
            i += 1

        self.libpyres.py_iofunc_register.argtypes = [ctypes.c_void_p, ctypes.py_object, ctypes.py_object, ctypes.c_void_p, ctypes.c_uint]
        self.libpyres.py_iofunc_register.restypes = ctypes.c_int
        res = self.libpyres.py_iofunc_register(self.iofunc_block_c, instance, method_name, register_array, i)
        if res < 0:
            raise(Exception(f"iofunc_register: {res}"))

    def iofunc_callback(self, id, *args, **kwargs):
        try:
            callback_method = getattr(self.instance, id)
            status, buf = callback_method(id, *args)
            n = 0 if buf is None else len(buf)
            return status, buf, n
        except Exception as e:
            print(f"Exception found: {str(e)}")

if __name__ == "__main__":
    pass