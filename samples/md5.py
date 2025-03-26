import ctypes
import hashlib
from resmgr import ResourceManager
from resmgr_handler import ResourceManagerHandler
from iofunc.iofunc import iofunc_ocb

class Md5_Ocb(iofunc_ocb):
    def __init__(self):
        self.total_len = 0
        self.md5_hash = hashlib.md5()
        super().__init__()

    def __format__(self, formatspec):
        return ""


class Md5Handler(ResourceManagerHandler):
    def __init__(self):
        ResourceManagerHandler.__init__(self)

    def ocb_alloc(self):
        return Md5_Ocb()
        
    def do_write(self, ocb, bbuf, nbytes):

        if (ocb.total_len == 0):
            self.md5_hash = hashlib.md5()

        buflen = len(bbuf)

        ocb.md5_hash.update(bbuf)
        ocb.offset += buflen

        while buflen < nbytes:
            chunklen = nbytes - buflen
            if chunklen > 4096:
                chunklen = 4096
            chunk = self.msgread(ocb, chunklen, buflen)
            if chunk is None:
                break
            chunklen = len(chunk)
            ocb.md5_hash.update(chunk)
            ocb.offset += chunklen
            buflen += chunklen

        return buflen, None

    def do_read(self, ocb, nbytes):

        if (ocb.offset == 0):
            return 0, None

        md5 = ocb.md5_hash.hexdigest().encode('utf-8')

        ocb.offset = 0
        
        n = len(md5)
        return n, md5

def main():
    md5handler = Md5Handler()
    md5handler.register_io_handler()
    
    res = ResourceManager()
    res.attach("/dev/md5", res._FTYPE_ANY, handler = md5handler)
    res.run_forever()

if __name__ == "__main__":
    main()