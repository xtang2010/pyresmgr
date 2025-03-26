#include "pyres.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

resmgr_block_t *py_resmgr_block_init(unsigned flag)
{
    resmgr_block_t *block;

    if ((block = malloc(sizeof(*block))) == NULL) {
        return NULL;
    }

    block->dispatch = dispatch_create();
    if (block->dispatch == NULL) {
        perror("dispatch_create");
        free(block);
        return NULL;
    }

    memset(&block->attr, 0, sizeof(block->attr));
    block->attr.flags = flag;
    block->attr.nparts_max = 10;
    block->attr.msg_max_size = 0;

    return block;
}

void py_resmgr_block_free(resmgr_block_t *block)
{
    free(block);
    return;
}

int py_resmgr_attach(resmgr_block_t *resblock, char *path, int ftype, iofunc_block_t *ioblock)
{
    int rmgid = resmgr_attach(resblock->dispatch, &resblock->attr, path, ftype, 0, 
                                &ioblock->connect_funcs, &ioblock->io_funcs, 
                                (iofunc_attr_t *)ioblock);
    if (rmgid == -1)
    {
        perror("resmgr_attach");
    	return -1;
    }

    resblock->resmgr_id = rmgid;

    return rmgid;
}

void py_resmgr_run(resmgr_block_t *resblock)
{
    resmgr_context_t         *ctp;

    ctp = resmgr_context_alloc(resblock->dispatch);
    if (ctp == NULL) {
        perror("resmgr_context_alloc");
        return;
    }

    while (1)
    {
        ctp = resmgr_block(ctp);
        if (ctp == NULL) {
            perror("resmgr_block");
            break;
        }

        resmgr_handler(ctp);
    }

    return;
}

size_t py_resmgr_msgget(void *ocb, void *buf, size_t size, size_t offset)
{
    pyres_ocb_t *o = (pyres_ocb_t *)ocb;

    return resmgr_msgread(o->ctp, buf, size, offset);
}

size_t py_resmgr_msgwrite(void *ocb, void *buf, size_t size, size_t offset)
{
    pyres_ocb_t *o = (pyres_ocb_t *)ocb;
    return resmgr_msgwrite(o->ctp, buf, size, offset);
}