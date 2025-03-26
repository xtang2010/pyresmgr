import ctypes
from resmgr_handler import ResourceManagerHandler

class ResourceManager():

    _FTYPE_ANY = 0
    _FTYPE_FILE = 1

    def __init__(self, flag = 0):
        self.resmgr_block_c = 0
        self.libpyres = ctypes.CDLL("libpyresmgr.so")
        self.libpyres.py_resmgr_block_init.argtypes = [ctypes.c_uint]
        self.libpyres.py_resmgr_block_init.restype  = ctypes.c_void_p
        self.resmgr_block_c = self.libpyres.py_resmgr_block_init(flag)
        if self.resmgr_block_c == 0:
            raise(Exception(f"libpyresmgr.resmgr_block_init({flag}) returned 0."))

    def attach(self, path, ftype, handler = None):
        self.resmgr_handler = handler if handler is not None else ResourceManagerHandler(self.libpyres)
        self.resmgr_handler.resmgr = self
        self.libpyres.py_resmgr_attach.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_uint, ctypes.c_void_p]
        self.libpyres.py_resmgr_attach.restype  = ctypes.c_int
        c_path = ctypes.c_char_p(path.encode('utf-8'))
        self.resmgr_id_c = self.libpyres.py_resmgr_attach(self.resmgr_block_c, c_path, ftype, self.resmgr_handler.iofunc.iofunc_block_c)
        
    def run_forever(self):
        self.libpyres.py_resmgr_run.argtypes = [ctypes.c_void_p]
        self.libpyres.py_resmgr_run.restype  = None
        self.libpyres.py_resmgr_run(self.resmgr_block_c)

