#include "pyres.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

iofunc_block_t *py_iofunc_block_init(int mode)
{
    iofunc_block_t *block;

    if ((block = malloc(sizeof(*block))) == NULL) {
        return NULL;
    }
    memset(block, 0, sizeof(*block));
   
    if (!Py_IsInitialized()) {
        printf("Not Initialized\n");
        Py_Initialize();
    }

    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyGILState_Release(gstate);

    if (mode == 0) {
        mode = 0666 | S_IFCHR;
    }
    iofunc_attr_init(&block->attr, mode, 0, 0);

    // hook up ocb_calloc / ocb_free
    block->mount_funcs.nfuncs = _IOFUNC_NFUNCS;
    block->mount_funcs.ocb_calloc = (void *)pyres_ocb_calloc;
    block->mount_funcs.ocb_free   = (void *)pyres_ocb_free;

    iofunc_mount_init(&block->mount, sizeof(block->mount));
    block->mount.conf |= IOFUNC_PC_ACL;
    block->mount.funcs = &block->mount_funcs;
    block->attr.mount  = &block->mount;

    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &block->connect_funcs, _RESMGR_IO_NFUNCS, &block->io_funcs);

    return block;
}

void py_iofunc_block_free(iofunc_block_t *block)
{
    free(block);
    return;
}

pyres_ocb_t * pyres_ocb_calloc(resmgr_context_t *ctp, iofunc_attr_t *attr)
{
    pyres_ocb_t *ocb;
    iofunc_block_t *block = (iofunc_block_t *)attr;

    if ((ocb = malloc(sizeof(*ocb))) == NULL) {
        return NULL;
    }

    memset(ocb, 0, sizeof(*ocb));
    ocb->o.attr = attr;
    ocb->block  = block;
    ocb->ctp    = ctp;

    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    if ((ocb->pOcb = PyObject_CallNoArgs(block->pResmgrHandlerOcbAlloc)) == NULL) {
        PyErr_Print();
        PyGILState_Release(gstate);
        free(ocb);
        return NULL;
    }

    if (PyObject_SetAttrString(ocb->pOcb, "ocb_c", PyLong_FromLong((long)ocb)) != 0) {
        PyErr_Print();
        PyGILState_Release(gstate);
        Py_DECREF(ocb->pOcb);
        free(ocb);
        return NULL;
    }

    PyGILState_Release(gstate);
    return ocb;
}

void pyres_ocb_free(pyres_ocb_t *ocb)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    if (!PyObject_Call(ocb->block->pResmgrHandlerOcbFree, Py_BuildValue("(O)", ocb->pOcb), NULL)) {
        PyErr_Print();
        PyErr_Clear();
    }

    PyGILState_Release(gstate);
    free(ocb);
}

int64_t pyres_ocb_get(iofunc_ocb_t *ocb, char *name)
{
    printf("pyres_ocb_get(%s)\n", name);
    if (strcmp(name, "attr") == 0) {
        return (int64_t)ocb->attr;
    } else if (strcmp(name, "ioflag") == 0) {
        return (int64_t)ocb->ioflag;
    } else if (strcmp(name, "sflag") == 0) {
        return (int64_t)ocb->sflag;
    } else if (strcmp(name, "flags") == 0) {
        return (int64_t)ocb->flags;
    } else if (strcmp(name, "offset") == 0) {
        return (int64_t)ocb->offset;
    }

    return -1;
}

int pyres_ocb_set(iofunc_ocb_t *ocb, char *name, int64_t value)
{
    //printf("pyres_ocb_set(%s, %ld)\n", name, value);
    if (strcmp(name, "offset") == 0) {
        ocb->offset = value;
        return 0;
    }

    return -1;
}

int py_iofunc_register(iofunc_block_t *block, PyObject *instance, char *callback, register_handler_t *register_map, int n)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    block->pResmgrHandlerInstance = instance;
    
    if (!(block->pResmgrHandlerOcbAlloc =  PyObject_GetAttrString(block->pResmgrHandlerInstance, "ocb_alloc"))) {
        PyErr_Print();
        PyGILState_Release(gstate);
        free(block);
        return -1;
    }

    if (!(block->pResmgrHandlerOcbFree  =  PyObject_GetAttrString(block->pResmgrHandlerInstance, "ocb_free"))) {
        PyErr_Print();
        PyGILState_Release(gstate);
        free(block);
        return -1;
    }

    //printf("iofunc_register: resmgrhandler=%p, alloc=%p, free=%p\n" , pInstance, block->pResmgrHandlerOcbAlloc, block->pResmgrHandlerOcbFree);
    PyGILState_Release(gstate);

    int id, i;
    if ((block->register_handler = malloc(sizeof(register_handler_t) * n)) == NULL) {
        return -1;
    }

    for (i = 0; i < n; i++) {
        id = register_map[i].id;
        if (id < _IO_CONNECT) {
            *((&block->connect_funcs.open) + (id - _IO_CONNECT_OPEN)) = (void *)py_iofunc_connect_handler;
        } else if (id >= _IO_READ && id <= _IO_UTIME64) {
            *((&block->io_funcs.read) + (id - _IO_READ)) = (void *)py_iofunc_io_handler;
        }
        block->register_handler[i] = register_map[i];
    }

    block->register_handler_len = n;

    return 0;
}

int py_iofunc_connect_handler(resmgr_context_t *ctp, struct _io_connect *msg, void *handle, void *extra)
{
    iofunc_block_t *block = (iofunc_block_t *)handle;
    void *func = NULL;
    int i, subtype = msg->subtype;

    for (i = 0; i < block->register_handler_len; i++) {
        if (subtype == block->register_handler[i].id) {
            func = block->register_handler[i].func;
        }
    }
    if (!func) {
        return _RESMGR_ERRNO(89);
    }

    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject *pKwargs, *pDict;
    
    pDict = PyDict_New();
    PyDict_SetItemString(pDict, "file_type",    Py_BuildValue("i", msg->file_type));
    PyDict_SetItemString(pDict, "ioflag",       Py_BuildValue("i", msg->ioflag));
    PyDict_SetItemString(pDict, "mode",         Py_BuildValue("i", msg->mode));
    PyDict_SetItemString(pDict, "sflag",        Py_BuildValue("i", msg->sflag));
    PyDict_SetItemString(pDict, "access",       Py_BuildValue("i", msg->access));
    PyDict_SetItemString(pDict, "dirfd",        Py_BuildValue("i", msg->dirfd));
    PyDict_SetItemString(pDict, "path_len",     Py_BuildValue("i", msg->path_len));
    PyDict_SetItemString(pDict, "eflag",        Py_BuildValue("i", msg->eflag));
    PyDict_SetItemString(pDict, "extra_type",   Py_BuildValue("i", msg->eflag));
    PyDict_SetItemString(pDict, "extra_len",    Py_BuildValue("i", msg->eflag));

    pKwargs = PyDict_New();
    PyDict_SetItemString(pKwargs, "connect",    Py_BuildValue("O", pDict));
    PyDict_SetItemString(pKwargs, "attr",       Py_BuildValue("z", handle));
    PyDict_SetItemString(pKwargs, "extra",      Py_BuildValue("z", extra));

    PyObject *pResult = PyObject_Call(func, Py_BuildValue("i", subtype), pKwargs);

    int   status;
    size_t size;
    char *buf;

    if (!PyArg_ParseTuple(pResult, "iz#i", &status, &buf, &size)) {
        PyErr_Print();
        PyErr_Clear();
        PyGILState_Release(gstate);
        return _RESMGR_ERRNO(ESRVRFAULT);
    }

    Py_DECREF(pDict);
    Py_DECREF(pKwargs);
    Py_DECREF(pResult);
    PyGILState_Release(gstate);

    _RESMGR_STATUS(ctp, status);
    return _RESMGR_PTR(ctp, buf, size);
}

int py_iofunc_io_handler(resmgr_context_t *ctp, io_msg_t *msg, pyres_ocb_t *ocb)
{
    iofunc_block_t *block = ocb->block;
    PyObject *func = NULL;
    int i, type = msg->i.type;

    //printf("income message type: %d\n", type);

    for (i = 0; i < block->register_handler_len; i++) {
        if (type == block->register_handler[i].id) {
            func = block->register_handler[i].func;
            break;
        }
    }
    if (!func) {
        return _RESMGR_ERRNO(89);
    }

    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();


    switch (msg->i.type) {
        case _IO_CONNECT:
            break;

        case _IO_READ:
        {
            io_read_t *rmsg = (io_read_t *)msg;
            int err;

            if ((err = iofunc_read_verify(ctp, rmsg, (iofunc_ocb_t *)ocb, NULL)) != 0) {
                return _RESMGR_ERRNO(err);    
            }

            PyObject  *pArgs, *pResult;

            pArgs = Py_BuildValue("OOi", block->pResmgrHandlerInstance, ocb->pOcb, rmsg->i.nbytes);
            if (pArgs == NULL) {
                PyErr_Print();
                PyErr_Clear();
                return _RESMGR_ERRNO(ENOMEM);
            }
            //Py_INCREF(ocb->pOcb);
            pResult = PyObject_Call(func, pArgs, NULL); 
            if (!pResult) {
                PyErr_Print();
                PyErr_Clear();
                PyGILState_Release(gstate);
                return _RESMGR_ERRNO(ESRVRFAULT);
            } 

            //set offset
            ocb->o.offset = PyLong_AsLong(PyObject_GetAttrString(ocb->pOcb, "offset"));

            int   status;
            size_t size;
            char *buf;

            if (!PyArg_ParseTuple(pResult, "iz#", &status, &buf, &size)) {
                PyErr_Print();
                PyErr_Clear();
                PyGILState_Release(gstate);
                return _RESMGR_ERRNO(ESRVRFAULT);
            }
            //printf("Back from python: status=%d, buf=%p, size=%ld\n", status, buf, size);

            Py_DECREF(pArgs);
            Py_DECREF(pResult);
            PyGILState_Release(gstate);

            _RESMGR_STATUS(ctp, status);
            return _RESMGR_PTR(ctp, buf, size);
            break;
        }
        
        case _IO_WRITE:
        {
            io_write_t *wmsg = (io_write_t *)msg;
            int err;

            if ((err = iofunc_write_verify(ctp, wmsg, (iofunc_ocb_t *)ocb, NULL)) != 0) {
                return _RESMGR_ERRNO(err);    
            }

            ctp->offset = sizeof(wmsg->i);
            char *buf = (char *)wmsg + ctp->offset;
            int  buflen = ctp->info.msglen - ctp->offset;

            PyObject  *pArgs, *pResult;

            pArgs = Py_BuildValue("OOy#i", block->pResmgrHandlerInstance, ocb->pOcb, buf, buflen, wmsg->i.nbytes);
            if (!pArgs) {
                PyErr_Print();
                PyErr_Clear();
                return _RESMGR_ERRNO(ENOMEM);
            }
            
            //Py_INCREF(ocb->pOcb);

            pResult = PyObject_Call(func, pArgs, NULL); 
            if (!pResult) {
                PyErr_Print();
                PyErr_Clear();
                PyGILState_Release(gstate);
                return _RESMGR_ERRNO(ESRVRFAULT);
            } 
            
            //set offset
            ocb->o.offset = PyLong_AsLong(PyObject_GetAttrString(ocb->pOcb, "offset"));

            int status, size;
            char *rbuf;
            if (!PyArg_ParseTuple(pResult, "iz#", &status, &rbuf, &size)) {
                PyErr_Print();
                PyErr_Clear();
                PyGILState_Release(gstate);
                return _RESMGR_ERRNO(ESRVRFAULT);
            }

            Py_DECREF(pArgs);
            Py_DECREF(pResult);

            PyGILState_Release(gstate);

            _RESMGR_STATUS(ctp, status);
            return _RESMGR_PTR(ctp, rbuf, size);
            break;
        }
    }
    return 0;
}

