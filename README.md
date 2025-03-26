# QNX Resource Manager in Python

QNX Resource Manager Framework is a very powerful tool to implement services on QNX. Originally it is implemented in C, these code here is to provide a python "bridge", so now you can create resource managers with in Python. The code is tested with QNX SDP 8.0.0, with Python 3.11 that comes with it.

## How to install

Grab the whole tree, first you need to build the libpyresmgr.so in pyresmgr/x86_64/so. I have include the binaries I built on QNX SDP 8.0, if you need your own architecture, or if you are using QNX SDP 7, feel free to rebuilt it, simply setup QNX development environment and type "make" in the directory. If your qnx have a different python version, you may also need to modify pyresmgr/common.mk

Create you QNX target machine, with "Python" option, bring it up.

Transfer the tree onto your target, before execution, you need to setup some environment so the code know where to load the libpyresmgr.so
```console
cd pyresmgr
export LD_LIBRAR_PATH=`pwd`/pyresmgr/x86_64/so
export PYTHONPATH=`pwd`
```
Now you can write some python code to create your own resource manager

## Write resource manager in python
In samples/ there are a couple of resource manager implemented as an sample.

### "Now" manager
With only ~30 lines of code, "now.py" implemented /dev/now, you can "cat /dev/now" to get the current time.

All you need to is create your own ResourceManagerHandler, with your own "do_read" handler.
```code
import ctypes
from datetime import datetime
from resmgr import ResourceManager
from resmgr_handler import ResourceManagerHandler
from iofunc.iofunc import iofunc_ocb

class NowHandler(ResourceManagerHandler):
    def __init__(self):
        ResourceManagerHandler.__init__(self)

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
```

### MD5 Manager
This resource manager create /dev/md5, you can "write" binaries into it, and "read" the md5 signature out (as md5file.py do)
Other then handle both "read" and "write" function, this resource manager also show how you can create an extended OCB, to store your own infomation.

## Others
The whole thing is a working in progress. I am happy to listen any further request
