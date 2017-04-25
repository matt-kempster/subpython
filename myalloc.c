/*! \file
 * Implementation of a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004-2010.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#include "myalloc.h"
#include "eval.h"


/*!
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated within
 * init_myalloc(), and then myalloc() and free() work against this pool of
 * memory that mem points to.
 */
int MEMORY_SIZE;
unsigned char *mem;

struct PoolHeader {
    /*! Includes both the size of this header and the data following it. */
    int obj_size;
    RefId ref;
};


/* The allocator uses an external "free-pointer" to track
 * where free memory starts.
 */
static unsigned char *freeptr;


/*!
 * This function initializes both the allocator state, and the memory pool.  It
 * must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory-pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example).
 */
void init_myalloc() {

    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests.
     */
    mem = (unsigned char *) malloc(MEMORY_SIZE);
    if (mem == 0) {
        fprintf(stderr,
                "init_myalloc: could not get %d bytes from the system\n",
		MEMORY_SIZE);
        abort();
    }

    freeptr = mem;
}


/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 */
unsigned char *myalloc(int size, RefId ref) {
    int requested = sizeof(struct PoolHeader) + size;
    if (freeptr + requested < mem + MEMORY_SIZE) {
        /* Write the header data to the bytes beginning at freeptr */
        struct PoolHeader *pool_header = (struct PoolHeader *) freeptr;
        pool_header->obj_size = requested;
        pool_header->ref = ref;

        /* The data region begins just after the header */
        unsigned char *resultptr = freeptr + sizeof(struct PoolHeader);

        /* Update the free pointer and return */
        freeptr += requested;
        return resultptr;
    } else {
            fprintf(stderr, "myalloc: cannot service request of size %d with"
                    " %lx bytes allocated\n", size, (freeptr - mem));
            return (unsigned char *) 0;
    }
}

void memdump() {
    unsigned char *curr = mem;
    unsigned char *curr_data;
    struct PoolHeader *curr_header;

    while (curr < freeptr) {
        curr_header = (struct PoolHeader *) curr;
        curr_data = curr + sizeof(struct PoolHeader);
        fprintf(stdout, "size %d; refId %d; data: ",
                curr_header->obj_size - sizeof(struct PoolHeader),
                curr_header->ref);
        for (int i = 0; i < curr_header->obj_size - sizeof(struct PoolHeader); i++) {
            fprintf(stdout, "%c", curr_data[i]);
        }
        fprintf(stdout, "\n");

        curr += curr_header->obj_size;
    }
}


/*!
 * Clean up the allocator state.
 * All this really has to do is free the user memory pool. This function mostly
 * ensures that the test program doesn't leak memory, so it's easy to check
 * if the allocator does.
 */
void close_myalloc() {
    free(mem);
}
