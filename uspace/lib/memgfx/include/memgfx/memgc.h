/*
 * Copyright (c) 2021 Jiri Svoboda
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup libmemgfx
 * @{
 */
/**
 * @file GFX memory backend
 */

#ifndef _MEMGFX_MEMGC_H
#define _MEMGFX_MEMGC_H

#include <errno.h>
#include <types/gfx/bitmap.h>
#include <types/gfx/context.h>
#include <types/gfx/ops/context.h>
#include <types/memgfx/memgc.h>

extern gfx_context_ops_t mem_gc_ops;

extern errno_t mem_gc_create(gfx_rect_t *, gfx_bitmap_alloc_t *,
    mem_gc_invalidate_cb_t, mem_gc_update_cb_t, void *, mem_gc_t **);
extern errno_t mem_gc_delete(mem_gc_t *);
extern void mem_gc_retarget(mem_gc_t *, gfx_rect_t *, gfx_bitmap_alloc_t *);
extern gfx_context_t *mem_gc_get_ctx(mem_gc_t *);

#endif

/** @}
 */
