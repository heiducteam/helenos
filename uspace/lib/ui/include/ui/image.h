/*
 * Copyright (c) 2020 Jiri Svoboda
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

/** @addtogroup libui
 * @{
 */
/**
 * @file Image
 */

#ifndef _UI_IMAGE_H
#define _UI_IMAGE_H

#include <errno.h>
#include <gfx/bitmap.h>
#include <gfx/coord.h>
#include <types/ui/control.h>
#include <types/ui/image.h>
#include <types/ui/resource.h>

extern errno_t ui_image_create(ui_resource_t *, gfx_bitmap_t *, gfx_rect_t *,
    ui_image_t **);
extern void ui_image_destroy(ui_image_t *);
extern ui_control_t *ui_image_ctl(ui_image_t *);
extern void ui_image_set_bmp(ui_image_t *, gfx_bitmap_t *, gfx_rect_t *);
extern void ui_image_set_rect(ui_image_t *, gfx_rect_t *);
extern void ui_image_set_flags(ui_image_t *, ui_image_flags_t);
extern errno_t ui_image_paint(ui_image_t *);

#endif

/** @}
 */
