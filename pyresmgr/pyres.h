#ifndef _PYRES_H_INCLUDE
#define _PYRES_H_INCLUDE

struct _pyres_ocb;
#define RESMGR_OCB_T struct _pyres_ocb
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/resmgr.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct _resmgr_block {
    dispatch_t    *dispatch;
    resmgr_attr_t attr;
    int           resmgr_id;
} resmgr_block_t;

typedef struct _register_handler {
    int           id;
    void         *func;
} register_handler_t;

typedef struct _iofunc_block {
    iofunc_attr_t          attr;
    resmgr_connect_funcs_t connect_funcs;
    resmgr_io_funcs_t      io_funcs;
    iofunc_mount_t         mount;
    iofunc_funcs_t         mount_funcs;
    PyObject               *pResmgrHandlerInstance;
    PyObject               *pResmgrHandlerOcbAlloc;
    PyObject               *pResmgrHandlerOcbFree;
    register_handler_t     *register_handler;
    int                    register_handler_len;
} iofunc_block_t;

typedef struct _pyres_ocb {
    iofunc_ocb_t           o;
    PyObject               *pOcb;
    resmgr_context_t       *ctp;
    iofunc_block_t         *block;
} pyres_ocb_t;

extern resmgr_block_t * py_resmgr_block_init(unsigned flag);
extern void             py_resmgr_block_free(resmgr_block_t *block);
extern iofunc_block_t * py_iofunc_block_init(int mode);
extern void             py_iofunc_block_free(iofunc_block_t *block);
extern int              py_resmgr_attach(resmgr_block_t *resblock, char *path, int ftype, iofunc_block_t *ioblock);
extern pyres_ocb_t    * pyres_ocb_calloc(resmgr_context_t *ctp, iofunc_attr_t *attr);
extern void             pyres_ocb_free(pyres_ocb_t *ocb);
extern int              py_iofunc_io_handler(resmgr_context_t *ctp, io_msg_t *msg, pyres_ocb_t *ocb);
extern int              py_iofunc_connect_handler(resmgr_context_t *ctp, struct _io_connect *msg, void *handle, void *extra);

#endif