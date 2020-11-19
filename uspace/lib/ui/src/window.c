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
 * @file Window
 */

#include <display.h>
#include <errno.h>
#include <gfx/bitmap.h>
#include <gfx/context.h>
#include <gfx/render.h>
#include <io/kbd_event.h>
#include <io/pos_event.h>
#include <mem.h>
#include <memgfx/memgc.h>
#include <stdlib.h>
#include <ui/control.h>
#include <ui/resource.h>
#include <ui/wdecor.h>
#include <ui/window.h>
#include "../private/control.h"
#include "../private/dummygc.h"
#include "../private/resource.h"
#include "../private/ui.h"
#include "../private/wdecor.h"
#include "../private/window.h"

static void dwnd_close_event(void *);
static void dwnd_focus_event(void *);
static void dwnd_kbd_event(void *, kbd_event_t *);
static void dwnd_pos_event(void *, pos_event_t *);
static void dwnd_unfocus_event(void *);

static display_wnd_cb_t dwnd_cb = {
	.close_event = dwnd_close_event,
	.focus_event = dwnd_focus_event,
	.kbd_event = dwnd_kbd_event,
	.pos_event = dwnd_pos_event,
	.unfocus_event = dwnd_unfocus_event
};

static void wd_close(ui_wdecor_t *, void *);
static void wd_move(ui_wdecor_t *, void *, gfx_coord2_t *);

static ui_wdecor_cb_t wdecor_cb = {
	.close = wd_close,
	.move = wd_move
};

static void ui_window_app_update(void *, gfx_rect_t *);

/** Initialize window parameters structure.
 *
 * Window parameters structure must always be initialized using this function
 * first.
 *
 * @param params Window parameters structure
 */
void ui_wnd_params_init(ui_wnd_params_t *params)
{
	memset(params, 0, sizeof(ui_wnd_params_t));
}

/** Create new window.
 *
 * @param ui User interface
 * @param params Window parameters
 * @param rwindow Place to store pointer to new window
 * @return EOK on success or an error code
 */
errno_t ui_window_create(ui_t *ui, ui_wnd_params_t *params,
    ui_window_t **rwindow)
{
	ui_window_t *window;
	display_info_t info;
	gfx_coord2_t pos;
	display_wnd_params_t dparams;
	display_window_t *dwindow = NULL;
	gfx_context_t *gc = NULL;
	ui_resource_t *res = NULL;
	ui_wdecor_t *wdecor = NULL;
	dummy_gc_t *dgc = NULL;
	errno_t rc;

	window = calloc(1, sizeof(ui_window_t));
	if (window == NULL)
		return ENOMEM;

	display_wnd_params_init(&dparams);
	dparams.rect = params->rect;

	if (ui->display != NULL) {
		rc = display_window_create(ui->display, &dparams, &dwnd_cb,
		    (void *) window, &dwindow);
		if (rc != EOK)
			goto error;

		if (params->placement != ui_wnd_place_default) {
			rc = display_get_info(ui->display, &info);
			if (rc != EOK)
				goto error;

			pos.x = 0;
			pos.y = 0;

			switch (params->placement) {
			case ui_wnd_place_default:
				assert(false);
			case ui_wnd_place_top_left:
				pos.x = info.rect.p0.x - params->rect.p0.x;
				pos.y = info.rect.p0.y - params->rect.p0.y;
				break;
			case ui_wnd_place_top_right:
				pos.x = info.rect.p1.x - params->rect.p1.x;
				pos.y = info.rect.p0.y - params->rect.p0.y;
				break;
			case ui_wnd_place_bottom_left:
				pos.x = info.rect.p0.x - params->rect.p0.x;
				pos.y = info.rect.p1.y - params->rect.p1.y;
				break;
			case ui_wnd_place_bottom_right:
				pos.x = info.rect.p1.x - params->rect.p1.x;
				pos.y = info.rect.p1.y - params->rect.p1.y;
				break;
			}

			rc = display_window_move(dwindow, &pos);
			if (rc != EOK)
				goto error;
		}

		rc = display_window_get_gc(dwindow, &gc);
		if (rc != EOK)
			goto error;
	} else {
		/* Needed for unit tests */
		rc = dummygc_create(&dgc);
		if (rc != EOK)
			goto error;

		gc = dummygc_get_ctx(dgc);
	}

	rc = ui_resource_create(gc, &res);
	if (rc != EOK)
		goto error;

	rc = ui_wdecor_create(res, params->caption, &wdecor);
	if (rc != EOK)
		goto error;

	ui_wdecor_set_rect(wdecor, &params->rect);
	ui_wdecor_set_cb(wdecor, &wdecor_cb, (void *) window);
	ui_wdecor_paint(wdecor);

	window->ui = ui;
	window->dwindow = dwindow;
	window->gc = gc;
	window->res = res;
	window->wdecor = wdecor;
	*rwindow = window;
	return EOK;
error:
	if (wdecor != NULL)
		ui_wdecor_destroy(wdecor);
	if (res != NULL)
		ui_resource_destroy(res);
	if (dgc != NULL)
		dummygc_destroy(dgc);
	if (dwindow != NULL)
		display_window_destroy(dwindow);
	free(window);
	return rc;
}

/** Destroy window.
 *
 * @param window Window or @c NULL
 */
void ui_window_destroy(ui_window_t *window)
{
	if (window == NULL)
		return;

	ui_control_destroy(window->control);
	ui_wdecor_destroy(window->wdecor);
	ui_resource_destroy(window->res);
	gfx_context_delete(window->gc);
	display_window_destroy(window->dwindow);
	free(window);
}

/** Add control to window.
 *
 * Only one control can be added to a window. If more than one control
 * is added, the results are undefined.
 *
 * @param fixed Fixed layout
 * @param control Control
 * @return EOK on success, ENOMEM if out of memory
 */
void ui_window_add(ui_window_t *window, ui_control_t *control)
{
	assert(window->control == NULL);

	window->control = control;
	control->elemp = (void *) window;
}

/** Remove control from window.
 *
 * @param window Window
 * @param control Control
 */
void ui_window_remove(ui_window_t *window, ui_control_t *control)
{
	assert(window->control == control);
	assert((ui_window_t *) control->elemp == window);

	window->control = NULL;
	control->elemp = NULL;
}

/** Resize/move window.
 *
 * Resize window to the dimensions of @a rect. If @a rect.p0 is not 0,0,
 * the top-left corner of the window will move on the screen accordingly.
 *
 * @param window Window
 * @param rect Rectangle
 *
 * @return EOK on success or an error code
 */
errno_t ui_window_resize(ui_window_t *window, gfx_rect_t *rect)
{
	gfx_coord2_t offs;
	gfx_rect_t nrect;
	errno_t rc;

	/*
	 * Move rect so that p0=0,0 - keep window's coordinate system origin
	 * locked to top-left corner of the window.
	 */
	offs = rect->p0;
	gfx_rect_rtranslate(&offs, rect, &nrect);

	/* dwindow can be NULL in case of unit tests */
	if (window->dwindow != NULL) {
		rc = display_window_resize(window->dwindow, &offs, &nrect);
		if (rc != EOK)
			return rc;
	}

	ui_wdecor_set_rect(window->wdecor, &nrect);
	ui_wdecor_paint(window->wdecor);
	return EOK;
}

/** Set window callbacks.
 *
 * @param window Window
 * @param cb Window decoration callbacks
 * @param arg Callback argument
 */
void ui_window_set_cb(ui_window_t *window, ui_window_cb_t *cb, void *arg)
{
	window->cb = cb;
	window->arg = arg;
}

/** Get UI resource from window.
 *
 * @param window Window
 * @return UI resource
 */
ui_resource_t *ui_window_get_res(ui_window_t *window)
{
	return window->res;
}

/** Get window GC.
 *
 * @param window Window
 * @return GC (relative to window)
 */
gfx_context_t *ui_window_get_gc(ui_window_t *window)
{
	return window->gc;
}

/** Get window application area GC
 *
 * @param window Window
 * @param rgc Place to store GC (relative to application area)
 * @return EOK on success or an error code
 */
errno_t ui_window_get_app_gc(ui_window_t *window, gfx_context_t **rgc)
{
	gfx_bitmap_params_t params;
	gfx_bitmap_alloc_t alloc;
	gfx_rect_t rect;
	mem_gc_t *memgc;
	errno_t rc;

	if (window->app_gc == NULL) {
		assert(window->app_bmp == NULL);

		gfx_bitmap_params_init(&params);

		/*
		 * The bitmap will have the same dimensions as the
		 * application rectangle, but start at 0,0.
		 */
		ui_window_get_app_rect(window, &rect);
		gfx_rect_rtranslate(&rect.p0, &rect, &params.rect);

		rc = gfx_bitmap_create(window->gc, &params, NULL,
		    &window->app_bmp);
		if (rc != EOK)
			return rc;

		rc = gfx_bitmap_get_alloc(window->app_bmp, &alloc);
		if (rc != EOK) {
			gfx_bitmap_destroy(window->app_bmp);
			return rc;
		}

		rc = mem_gc_create(&params.rect, &alloc, ui_window_app_update,
		    (void *) window, &memgc);
		if (rc != EOK) {
			gfx_bitmap_destroy(window->app_bmp);
			return rc;
		}

		window->app_gc = mem_gc_get_ctx(memgc);
	}

	*rgc = window->app_gc;
	return EOK;
}

/** Get window application rectangle
 *
 * @param window Window
 * @param rect Place to store application rectangle
 */
void ui_window_get_app_rect(ui_window_t *window, gfx_rect_t *rect)
{
	ui_wdecor_geom_t geom;

	ui_wdecor_get_geom(window->wdecor, &geom);
	*rect = geom.app_area_rect;
}

/** Paint window
 *
 * @param window Window
 * @return EOK on success or an error code
 */
errno_t ui_window_paint(ui_window_t *window)
{
	return ui_window_send_paint(window);
}

/** Handle window close event. */
static void dwnd_close_event(void *arg)
{
	ui_window_t *window = (ui_window_t *) arg;

	ui_window_send_close(window);
}

/** Handle window focus event. */
static void dwnd_focus_event(void *arg)
{
	ui_window_t *window = (ui_window_t *) arg;

	if (window->wdecor != NULL) {
		ui_wdecor_set_active(window->wdecor, true);
		ui_wdecor_paint(window->wdecor);
	}

	ui_window_send_focus(window);
}

/** Handle window keyboard event */
static void dwnd_kbd_event(void *arg, kbd_event_t *kbd_event)
{
	ui_window_t *window = (ui_window_t *) arg;

	(void) window;
	ui_window_send_kbd(window, kbd_event);
}

/** Handle window position event */
static void dwnd_pos_event(void *arg, pos_event_t *event)
{
	ui_window_t *window = (ui_window_t *) arg;

	/* Make sure we don't process events until fully initialized */
	if (window->wdecor == NULL)
		return;

	ui_wdecor_pos_event(window->wdecor, event);
	ui_window_send_pos(window, event);
}

/** Handle window unfocus event. */
static void dwnd_unfocus_event(void *arg)
{
	ui_window_t *window = (ui_window_t *) arg;

	if (window->wdecor != NULL) {
		ui_wdecor_set_active(window->wdecor, false);
		ui_wdecor_paint(window->wdecor);
	}

	ui_window_send_unfocus(window);
}

/** Window decoration requested window closure.
 *
 * @param wdecor Window decoration
 * @param arg Argument (demo)
 */
static void wd_close(ui_wdecor_t *wdecor, void *arg)
{
	ui_window_t *window = (ui_window_t *) arg;

	ui_window_send_close(window);
}

/** Window decoration requested window move.
 *
 * @param wdecor Window decoration
 * @param arg Argument (demo)
 * @param pos Position where the title bar was pressed
 */
static void wd_move(ui_wdecor_t *wdecor, void *arg, gfx_coord2_t *pos)
{
	ui_window_t *window = (ui_window_t *) arg;

	(void) display_window_move_req(window->dwindow, pos);
}

/** Send window close event.
 *
 * @param window Window
 */
void ui_window_send_close(ui_window_t *window)
{
	if (window->cb != NULL && window->cb->close != NULL)
		window->cb->close(window, window->arg);
}

/** Send window focus event.
 *
 * @param window Window
 */
void ui_window_send_focus(ui_window_t *window)
{
	if (window->cb != NULL && window->cb->focus != NULL)
		window->cb->focus(window, window->arg);
}

/** Send window keyboard event.
 *
 * @param window Window
 */
void ui_window_send_kbd(ui_window_t *window, kbd_event_t *kbd)
{
	if (window->cb != NULL && window->cb->kbd != NULL)
		window->cb->kbd(window, window->arg, kbd);
}

/** Send window paint event.
 *
 * @param window Window
 */
errno_t ui_window_send_paint(ui_window_t *window)
{
	if (window->cb != NULL && window->cb->paint != NULL)
		return window->cb->paint(window, window->arg);
	else
		return ui_window_def_paint(window);
}

/** Send window position event.
 *
 * @param window Window
 */
void ui_window_send_pos(ui_window_t *window, pos_event_t *pos)
{
	if (window->cb != NULL && window->cb->pos != NULL)
		window->cb->pos(window, window->arg, pos);
	else
		ui_window_def_pos(window, pos);
}

/** Send window unfocus event.
 *
 * @param window Window
 */
void ui_window_send_unfocus(ui_window_t *window)
{
	if (window->cb != NULL && window->cb->unfocus != NULL)
		window->cb->unfocus(window, window->arg);
}

/** Default window paint routine.
 *
 * @param window Window
 * @return EOK on success or an error code
 */
errno_t ui_window_def_paint(ui_window_t *window)
{
	gfx_rect_t app_rect;
	errno_t rc;

	rc = gfx_set_color(window->gc, window->res->wnd_face_color);
	if (rc != EOK)
		return rc;

	ui_window_get_app_rect(window, &app_rect);

	rc = gfx_fill_rect(window->gc, &app_rect);
	if (rc != EOK)
		return rc;

	if (window->control != NULL)
		return ui_control_paint(window->control);

	return EOK;
}

/** Default window position event routine.
 *
 * @param window Window
 * @return EOK on success or an error code
 */
void ui_window_def_pos(ui_window_t *window, pos_event_t *pos)
{
	if (window->control != NULL)
		ui_control_pos_event(window->control, pos);
}

/** Application area update callback
 *
 * @param arg Argument (ui_window_t *)
 * @param rect Rectangle to update
 */
static void ui_window_app_update(void *arg, gfx_rect_t *rect)
{
	ui_window_t *window = (ui_window_t *) arg;
	gfx_rect_t arect;

	ui_window_get_app_rect(window, &arect);

	/* Render bitmap rectangle inside the application area */
	(void) gfx_bitmap_render(window->app_bmp, rect, &arect.p0);
}

/** @}
 */
