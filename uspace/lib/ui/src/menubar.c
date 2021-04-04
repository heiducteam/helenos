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

/** @addtogroup libui
 * @{
 */
/**
 * @file Menu bar
 */

#include <adt/list.h>
#include <errno.h>
#include <gfx/color.h>
#include <gfx/context.h>
#include <gfx/render.h>
#include <gfx/text.h>
#include <io/pos_event.h>
#include <stdlib.h>
#include <str.h>
#include <ui/control.h>
#include <ui/paint.h>
#include <ui/menu.h>
#include <ui/menubar.h>
#include "../private/menubar.h"
#include "../private/resource.h"

enum {
	menubar_hpad = 4,
	menubar_vpad = 4,
	menubar_hpad_text = 1,
	menubar_vpad_text = 0
};

static void ui_menu_bar_ctl_destroy(void *);
static errno_t ui_menu_bar_ctl_paint(void *);
static ui_evclaim_t ui_menu_bar_ctl_pos_event(void *, pos_event_t *);
static void ui_menu_bar_ctl_unfocus(void *);

/** Menu bar control ops */
ui_control_ops_t ui_menu_bar_ops = {
	.destroy = ui_menu_bar_ctl_destroy,
	.paint = ui_menu_bar_ctl_paint,
	.pos_event = ui_menu_bar_ctl_pos_event,
	.unfocus = ui_menu_bar_ctl_unfocus
};

/** Create new menu bar.
 *
 * @param res UI resource
 * @param rmbar Place to store pointer to new menu bar
 * @return EOK on success, ENOMEM if out of memory
 */
errno_t ui_menu_bar_create(ui_resource_t *res, ui_menu_bar_t **rmbar)
{
	ui_menu_bar_t *mbar;
	errno_t rc;

	mbar = calloc(1, sizeof(ui_menu_bar_t));
	if (mbar == NULL)
		return ENOMEM;

	rc = ui_control_new(&ui_menu_bar_ops, (void *) mbar, &mbar->control);
	if (rc != EOK) {
		free(mbar);
		return rc;
	}

	mbar->res = res;
	list_initialize(&mbar->menus);
	*rmbar = mbar;
	return EOK;
}

/** Destroy menu bar
 *
 * @param mbar Menu bar or @c NULL
 */
void ui_menu_bar_destroy(ui_menu_bar_t *mbar)
{
	ui_menu_t *menu;

	if (mbar == NULL)
		return;

	/* Destroy menus */
	menu = ui_menu_first(mbar);
	while (menu != NULL) {
		ui_menu_destroy(menu);
		menu = ui_menu_first(mbar);
	}

	ui_control_delete(mbar->control);
	free(mbar);
}

/** Get base control from menu bar.
 *
 * @param mbar Menu bar
 * @return Control
 */
ui_control_t *ui_menu_bar_ctl(ui_menu_bar_t *mbar)
{
	return mbar->control;
}

/** Set menu bar rectangle.
 *
 * @param mbar Menu bar
 * @param rect New menu bar rectangle
 */
void ui_menu_bar_set_rect(ui_menu_bar_t *mbar, gfx_rect_t *rect)
{
	mbar->rect = *rect;
}

/** Paint menu bar.
 *
 * @param mbar Menu bar
 * @return EOK on success or an error code
 */
errno_t ui_menu_bar_paint(ui_menu_bar_t *mbar)
{
	gfx_text_fmt_t fmt;
	gfx_coord2_t pos;
	gfx_coord2_t tpos;
	gfx_rect_t rect;
	gfx_color_t *bg_color;
	ui_menu_t *menu;
	const char *caption;
	gfx_coord_t width;
	gfx_coord_t hpad;
	gfx_coord_t vpad;
	errno_t rc;

	/* Paint menu bar background */

	rc = gfx_set_color(mbar->res->gc, mbar->res->wnd_face_color);
	if (rc != EOK)
		goto error;

	rc = gfx_fill_rect(mbar->res->gc, &mbar->rect);
	if (rc != EOK)
		goto error;

	if (mbar->res->textmode) {
		hpad = menubar_hpad_text;
		vpad = menubar_vpad_text;
	} else {
		hpad = menubar_hpad;
		vpad = menubar_vpad;
	}

	pos = mbar->rect.p0;

	gfx_text_fmt_init(&fmt);
	fmt.halign = gfx_halign_left;
	fmt.valign = gfx_valign_top;

	menu = ui_menu_first(mbar);
	while (menu != NULL) {
		caption = ui_menu_caption(menu);
		width = gfx_text_width(mbar->res->font, caption) + 2 * hpad;
		tpos.x = pos.x + hpad;
		tpos.y = pos.y + vpad;

		rect.p0 = pos;
		rect.p1.x = rect.p0.x + width;
		rect.p1.y = mbar->rect.p1.y;

		if (menu == mbar->selected) {
			fmt.color = mbar->res->wnd_sel_text_color;
			bg_color = mbar->res->wnd_sel_text_bg_color;
		} else {
			fmt.color = mbar->res->wnd_text_color;
			bg_color = mbar->res->wnd_face_color;
		}

		rc = gfx_set_color(mbar->res->gc, bg_color);
		if (rc != EOK)
			goto error;

		rc = gfx_fill_rect(mbar->res->gc, &rect);
		if (rc != EOK)
			goto error;

		rc = gfx_puttext(mbar->res->font, &tpos, &fmt, caption);
		if (rc != EOK)
			goto error;

		pos.x += width;
		menu = ui_menu_next(menu);
	}

	rc = gfx_update(mbar->res->gc);
	if (rc != EOK)
		goto error;

	return EOK;
error:
	return rc;
}

/** Select or deselect menu from menu bar.
 *
 * Select @a menu. If @a menu is @c NULL or it is already selected,
 * then select none.
 *
 * @param mbar Menu bar
 * @param pos Position (top-left corner) of menu bar entry
 * @param menu Menu to select (or deselect if selected) or @c NULL
 */
void ui_menu_bar_select(ui_menu_bar_t *mbar, gfx_coord2_t *pos,
    ui_menu_t *menu)
{
	gfx_coord2_t spos;
	ui_menu_t *old_menu;

	old_menu = mbar->selected;

	if (mbar->selected != menu)
		mbar->selected = menu;
	else
		mbar->selected = NULL;

	/* Need to clear the menu has just been closed */
	if (old_menu != NULL)
		(void) ui_menu_unpaint(old_menu);

	(void) ui_menu_bar_paint(mbar);

	if (mbar->selected != NULL) {
		/* Cache position of selected entry */
		mbar->sel_pos = *pos;

		/* Position menu under selected menu bar entry */
		spos.x = pos->x;
		spos.y = mbar->rect.p1.y;

		(void) ui_menu_paint(mbar->selected, &spos);
	}
}

/** Handle menu bar position event.
 *
 * @param mbar Menu bar
 * @param pos_event Position event
 * @return @c ui_claimed iff the event is claimed
 */
ui_evclaim_t ui_menu_bar_pos_event(ui_menu_bar_t *mbar, pos_event_t *event)
{
	gfx_coord2_t pos;
	gfx_coord2_t spos;
	gfx_rect_t rect;
	ui_menu_t *menu;
	const char *caption;
	gfx_coord_t width;
	gfx_coord_t hpad;
	gfx_coord2_t ppos;
	ui_evclaim_t claimed;

	ppos.x = event->hpos;
	ppos.y = event->vpos;

	if (mbar->res->textmode) {
		hpad = menubar_hpad_text;
	} else {
		hpad = menubar_hpad;
	}

	pos = mbar->rect.p0;

	menu = ui_menu_first(mbar);
	while (menu != NULL) {
		caption = ui_menu_caption(menu);
		width = gfx_text_width(mbar->res->font, caption) + 2 * hpad;

		rect.p0 = pos;
		rect.p1.x = rect.p0.x + width;
		rect.p1.y = mbar->rect.p1.y;

		/* Check if press is inside menu bar entry */
		if (event->type == POS_PRESS &&
		    gfx_pix_inside_rect(&ppos, &rect)) {
			ui_menu_bar_select(mbar, &pos, menu);
			return ui_claimed;
		}

		if (menu == mbar->selected) {
			/* Open menu is positioned below menu bar entry */
			spos.x = pos.x;
			spos.y = mbar->rect.p1.y;

			ui_menu_get_rect(menu, &spos, &rect);

			claimed = ui_menu_pos_event(menu, &spos, event);
			if (claimed == ui_claimed)
				return ui_claimed;
		}

		pos.x += width;
		menu = ui_menu_next(menu);
	}

	return ui_unclaimed;
}

/** Handle menu bar window unfocus notification.
 *
 * @param mbar Menu bar
 */
void ui_menu_bar_unfocus(ui_menu_bar_t *mbar)
{
	ui_menu_bar_select(mbar, NULL, NULL);
}

/** Destroy menu bar control.
 *
 * @param arg Argument (ui_menu_bar_t *)
 */
void ui_menu_bar_ctl_destroy(void *arg)
{
	ui_menu_bar_t *mbar = (ui_menu_bar_t *) arg;

	ui_menu_bar_destroy(mbar);
}

/** Paint menu bar control.
 *
 * @param arg Argument (ui_menu_bar_t *)
 * @return EOK on success or an error code
 */
errno_t ui_menu_bar_ctl_paint(void *arg)
{
	ui_menu_bar_t *mbar = (ui_menu_bar_t *) arg;

	return ui_menu_bar_paint(mbar);
}

/** Handle menu bar control position event.
 *
 * @param arg Argument (ui_menu_bar_t *)
 * @param pos_event Position event
 * @return @c ui_claimed iff the event is claimed
 */
ui_evclaim_t ui_menu_bar_ctl_pos_event(void *arg, pos_event_t *event)
{
	ui_menu_bar_t *mbar = (ui_menu_bar_t *) arg;

	return ui_menu_bar_pos_event(mbar, event);
}

/** Handle menu bar control window unfocus notification.
 *
 * @param arg Argument (ui_menu_bar_t *)
 */
void ui_menu_bar_ctl_unfocus(void *arg)
{
	ui_menu_bar_t *mbar = (ui_menu_bar_t *) arg;

	ui_menu_bar_unfocus(mbar);
}

/** @}
 */