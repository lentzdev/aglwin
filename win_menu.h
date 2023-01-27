/* AGLWIN video windows manager
 * Copyright (C) 1990-2001 Arjen G. Lentz
 *
 * This file is part of AGLWIN.
 * AGLWIN is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


/* win_menu.h - pulldown/popup/bar menufunctions */
#ifndef __WIN_MENU_DEF_
#define __WIN_MENU_DEF_


#ifdef __cplusplus
extern "C" {
#endif


struct _menu_item {
	char *text;			/* option text to be displayed	     */
	word  key;			/* selection key for each option     */
	byte  flags;			/* action flags (see OPT_xxx below)  */
#ifdef __PROTO__
	int cdecl (*function)(struct _win_menu *mp,word keycode);   /* func */
#else
	int cdecl (*function)();
#endif
	struct _win_menu *menu; 				     /* menu */
};


struct _select_item {
	char   *text;			/* option text to be displayed	     */
	byte	flags;			/* flags - only OPT_REFRESH valid    */
};


struct _hotkey_item {
	word key;			/* selection key for each option     */
	byte flags;			/* action flags (see OPT_xxx below)  */
#ifdef __PROTO__
	int cdecl (*function)(struct _win_menu *mp,word keycode);   /* func */
#else
	int cdecl (*function)();
#endif
	struct _win_menu *menu; 				     /* menu */
};


typedef struct _win_menu {
	byte	menu_type;	/* ------- type of menu (see MENU_xxx below) */
	byte	menu_border;		/* border line type (single/double)  */
	byte	menu_left,		/* left offset of window in screen   */
		menu_top;		/* top	 ...			     */
	char   *menu_title;		/* title of popup/pulldown window    */
	WIN_IDX menu_win;		/* internal: window handle	     */
	struct _win_menu *menu_parent;	/*	     ptr to parent menu      */
	byte	atr_normal,		/* attributes: normal text lines     */
		atr_hotnormal,		/*	       hotkey character      */
		atr_select,		/*	       selection bar	     */
		atr_hotselect,		/*	       hotkey in select. bar */
		atr_inactive,		/*	       inactive item	     */
		atr_border,		/*	       border/seperate lines */
		atr_title;		/*	       title string	     */
#ifdef __PROTO__
		void cdecl (*help)(void *object, word item);   /* help func */
#else
		void cdecl (*help)();
#endif
	byte	num_items;	/* ------- number of items in option lists   */
	byte	opt_last;		/* the last selected menu item	     */
	struct _menu_item *item;	/* array of all menu items	     */
} WIN_MENU;


typedef struct {
	word	num_items;		/* number of hotkeys		     */
	struct _hotkey_item *item;	/* array of all hotkey items	     */
} WIN_HOTKEYS;


typedef struct _win_select {
	byte	select_flags;  /* ------- flags - only noopen/noclose valid */
	byte	select_border;		/* border line type (single/double)  */
	byte	select_left,		/* left offset of window in screen   */
		select_top,		/* top	  ...			     */
		select_width,		/* width of items in window	     */
		select_depth;		/* depth of item section of window   */
	char   *select_title;		/* title of popup/pulldown window    */
	WIN_IDX select_win;		/* internal: window handle	     */
	byte	atr_normal,		/* attributes: normal text lines     */
		atr_select,		/*	       selection bar	     */
		atr_border,		/*	       border lines	     */
		atr_title,		/*	       title string	     */
		atr_scrollbar,		/*	       scrollbar main stuff  */
		atr_scrollselect;	/*	       scrollbar marker char */
	word	num_items;		/* number of items in selection list */
	word	opt_last;		/* the last selected menu item	     */
	word	opt_topline;		/* menu item on top line of window   */
	byte	opt_scrollbar;		/* line offset of scrollbar marker   */
	struct _select_item *item;	/* array of all selection items      */
#ifdef __PROTO__
	int cdecl (*init)(struct _win_select *sp, word item);	 /* text init func */
	int cdecl (*action)(struct _win_select *sp, word keycode);     /* function */
#else
	int cdecl (*init)();
	int cdecl (*action)();
#endif
} WIN_SELECT;


#define MENU_BAR	       0	/* menu types: horizontal select.bar */
#define MENU_POPUP	       1	/*	       vertical popup menu   */
#define MENU_PULLDOWN	       1	/*	       like popup, pulldown  */
#define MENU_TYPEMASK	    0x07	/* bits used for menutype, mask open */
#define MENU_DELAYIMMEDIATE 0x08	/* don't do immediate at menu start  */
#define MENU_NOOPEN	    0x10	/* don't open window upon entry      */
#define MENU_NOCLOSE	    0x20	/* don't close window upon exit      */
#define MENU_SHUTTOTOP	    0x40	/* shut all but top menu	     */
#define MENU_SHUTALL	    0x80	/* shut all menu windows	     */

#define SELECT_NONE	    0x00	/* nothing			     */
#define SELECT_NOOPEN	    0x10	/* don't open window upon entry      */
#define SELECT_NOCLOSE	    0x20	/* don't close window upon exit      */
#define SELECT_SCROLLBAR    0x40	/* show scrollbar in selection menu  */

#define OPT_NONE	    0x00	/*	    nothing, just do options */
#define OPT_SHUTONE	    0x01	/* options: shut this menu level     */
#define OPT_SHUTTOTOP	    0x02	/*	    shut all but top menu    */
#define OPT_SHUTALL	    0x04	/*	    shut all menu windows    */
#define OPT_HIDE	    0x08	/*	    hide all menu windows    */
#define OPT_DESELECT	    0x10	/* remove selection bar 	     */
#define OPT_IMMEDIATE	    0x40	/* options: auto-exe when moved to   */
#define OPT_REFRESH	    0x80	/* set by sub: redisplay upon return */


#ifdef	__cplusplus
}
#endif


#endif/*__WIN_MENU_DEF_*/


/* end of win_menu.h */
