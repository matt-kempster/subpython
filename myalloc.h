/*! \file
 * Declarations for a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004-2009.
 * All rights reserved.
 */

#ifndef MYALLOC_H
#define MYALLOC_H

#include "eval.h"

/*! Specifies the size of the memory pool the allocator has to work with. */
extern int MEMORY_SIZE;


/* Initializes allocator state, and memory pool state too. */
void init_myalloc();


/* Attempt to allocate a chunk of memory of "size" bytes. */
void *myalloc(int size, RefId ref);


/* Print all the information in the pool. */
void memdump();


/* Clean up the allocator and memory pool state. */
void close_myalloc();

#endif /* MYALLOC_H */
