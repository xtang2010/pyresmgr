# A QNX resource manager in python, it regist /dev/now, and when "cat", it will return current time.
#

import ctypes
from datetime import datetime
from resmgr import ResourceManager
from resmgr_handler import ResourceManagerHandler
from iofunc.iofunc import iofunc_ocb

class NowOcb(iofunc_ocb):
    def __init__(self):
        iofunc_ocb.__init__(self)

class NowHandler(ResourceManagerHandler):
    def __init__(self):
        ResourceManagerHandler.__init__(self)

    #def ocb_alloc(self):
    #    return NowOcb()
    
    def do_read(self, ocb, nbytes):
        
        if ocb.offset != 0:
            return 0, None
        
        t = datetime.strftime(datetime.now(), "%Y-%m-%d %H:%M:%S\n").encode('utf-8')
        n = len(t)
        ocb.offset += n
        
        return n, t

def main():
    nowhandler = NowHandler()
    nowhandler.register_io_handler()
    
    res = ResourceManager()
    res.attach("/dev/now", res._FTYPE_ANY, handler = nowhandler)
    res.run_forever()

if __name__ == "__main__":
    main()