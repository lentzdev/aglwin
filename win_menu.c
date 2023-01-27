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


/* win_menu.c - pulldown/popup/bar menufunctions */
#include <string.h>
#include <ctype.h>
#ifdef __MSDOS__
#include <alloc.h>
#include <mem.h>
#endif
#ifdef __PROTO__
#include <stdlib.h>
#endif
#include "window.h"


struct _menu_info {
	WIN_HOTKEYS *hotkeys;
	word	 key;
	byte	 flags;
#ifdef __PROTO__
	int cdecl (*function)(struct _win_menu *mp,word keycode);
#else
	int cdecl (*function)();
#endif
	struct _win_menu *menu;
};


#ifdef __PROTO__
static void near _win_fast menu_show	(WIN_MENU *mp, byte deselect);
static int  near _win_fast menu_next	(WIN_MENU *mp, byte mousekey);
static int  near _win_fast menu_prev	(WIN_MENU *mp, byte mousekey);
static int  near _win_fast menu_action	(WIN_MENU *mp, struct _menu_info *mi);
static void near _win_fast menu_new	(WIN_MENU *mp, struct _menu_info *mi);
static void near _win_fast select_show	(WIN_SELECT *sp);
#else
static void near _win_fast menu_show();
static int  near _win_fast menu_next();
static int  near _win_fast menu_prev();
static int  near _win_fast menu_action();
static void near _win_fast menu_new();
static void near _win_fast select_show();
#endif


static void near _win_fast menu_show(mp,deselect)
WIN_MENU *mp;
byte deselect;
{
	byte i, hor, ver, len, offset;
	word attrib;
	SCR_PTR win_ptr;
	char *p, *p2;

	win_setwp(mp->menu_win);
	hor = wp->cur_left;
	ver = wp->cur_top;

	for (i = 0; i < mp->num_items; i++) {
	    if (mp->item[i].flags & OPT_REFRESH) {
	       if (mp->item[i].text != NULL) {		 /* normal text line */
		  attrib = ((word) ((mp->opt_last == i && !deselect) ?
				    mp->atr_select :
				    (mp->item[i].key ?
				     mp->atr_normal : mp->atr_inactive))) << 8;
		  win_ptr = wp->win_buf + ver * wp->win_horlen + hor;
		  if ((mp->menu_type & MENU_TYPEMASK) != MENU_BAR)
		     win_setbuf(win_ptr,attrib | (byte) ' ',wp->cur_horlen);
		  for (len = 0; mp->item[i].text[len]; len++)
		      win_ptr[len] = attrib | (byte) mp->item[i].text[len];

		  if (mp->item[i].key < 0x07f && mp->item[i].key > 0x020) {
		     p = strchr(mp->item[i].text,inl_toupper(mp->item[i].key));
		     p2 = strchr(mp->item[i].text,inl_tolower(mp->item[i].key));
		     if (p2 != NULL && (p == NULL || p2 < p))
			p = p2;
		     if (p != NULL) {
			offset = p - mp->item[i].text;
			attrib = ((word) ((mp->opt_last == i && !deselect) ?
					  mp->atr_hotselect :
					  mp->atr_hotnormal)) << 8;
			win_ptr[offset] = attrib | (byte) mp->item[i].text[offset];
		     }
		  }
		  scr_update(wp,hor,ver,
			     (mp->menu_type & MENU_TYPEMASK) != MENU_BAR ?
			     wp->cur_horlen : len);
	       }
	       else if ((mp->menu_type & MENU_TYPEMASK) != MENU_BAR) {
		  win_setattrib(mp->menu_win,mp->atr_border);
		  win_line(mp->menu_win, 1, ver + 1,
			   wp->win_horlen, ver + 1, mp->menu_border);
	       }
	       else continue;

	       mp->item[i].flags &= ~OPT_REFRESH;
	    }

	    if ((mp->menu_type & MENU_TYPEMASK) == MENU_BAR)
	       hor += strlen(mp->item[i].text);
	    else
	       ver++;
	}

	if (mp->help != NULL)
	   (*mp->help)(mp,mp->opt_last);
}/*menu_show()*/


static int near _win_fast menu_next(mp, mousekey)
WIN_MENU *mp;
byte mousekey;
{
	byte newselect = mp->opt_last;

	do {
	   if (newselect >= (mp->num_items - 1)) {
	      if (mousekey) return (0);
	      newselect = 0;
	   }
	   else
	      newselect++;
	   if (newselect == mp->opt_last) return (0);
	} while (!mp->item[newselect].key);

	mp->item[mp->opt_last].flags |= OPT_REFRESH;
	mp->item[newselect].flags |= OPT_REFRESH;
	mp->opt_last = newselect;

	return (1);
}/*menu_next()*/


static int near _win_fast menu_prev(mp, mousekey)
WIN_MENU *mp;
byte mousekey;
{
	byte newselect = mp->opt_last;

	do {
	   if (!newselect) {
	      if (mousekey) return (0);
	      newselect = mp->num_items - 1;
	   }
	   else
	      newselect--;
	   if (newselect == mp->opt_last) return (0);
	} while (!mp->item[newselect].key);

	mp->item[mp->opt_last].flags |= OPT_REFRESH;
	mp->item[newselect].flags |= OPT_REFRESH;
	mp->opt_last = newselect;

	return (1);
}/*menu_prev()*/


static int near _win_fast menu_action(mp, mi)
WIN_MENU *mp;
struct _menu_info *mi;
{	/* returns: 0 = reprint/select, 1=immediate+0,	2 = exit this menu */
again:	if (mi->flags & OPT_SHUTONE) {
	   mi->flags &= ~OPT_SHUTONE;
	   return (2);
	}
	if ((mi->flags & OPT_SHUTTOTOP) && mp != NULL && mp->menu_parent != NULL)
	   return (2);
	if ((mi->flags & OPT_SHUTALL) && mp != NULL)
	   return (2);

	if ((mi->flags & OPT_DESELECT) && mp != NULL) {
	   mp->item[mp->opt_last].flags |= OPT_REFRESH;
	   menu_show(mp,1);
	}
	if (mi->function != NULL) {
	   int res;

	   if (mi->flags & OPT_HIDE) {
	      WIN_IDX *menu_stack;
	      WIN_MENU *curmenu;
	      byte menu_st_top;

	      menu_stack = (WIN_IDX *) malloc(num_windows * sizeof (WIN_IDX));
	      for (menu_st_top = 0, curmenu = mp;
		   curmenu != NULL; curmenu = curmenu->menu_parent) {
		  menu_stack[menu_st_top++] = curmenu->menu_win;
		  win_hide(curmenu->menu_win);
	      }
	      res = (*mi->function)(mp,mi->key);
	      while (menu_st_top)
		    win_settop(menu_stack[--menu_st_top]);
	      free(menu_stack);
	   }
	   else
	      res = (*mi->function)(mp,mi->key);
	   mi->function = NULL;
	   if (mp!=NULL) win_settop(mp->menu_win);
	   if (res) {			   /* exit menu if actionfunc() != 0 */
	      mi->flags |= OPT_SHUTALL;    /* drop out of this menu tree     */
	      mi->menu = NULL;		   /* ignore menu if it was there... */
	      return (2);
	   }
	}

	if (mi->menu != NULL) {
	   mi->menu->menu_parent = mp;		/* tell him who his daddy is */
	   menu_new(mi->menu,mi);
	   goto again;
	}

	if ((mi->flags & OPT_DESELECT) && mp!=NULL) {
	   mp->item[mp->opt_last].flags |= OPT_REFRESH;
	   menu_show(mp,0);
	}

	return ((mi->flags & OPT_IMMEDIATE) ? 1 : 0);
}/*menu_action()*/


static void near _win_fast menu_new(mp, mi)
WIN_MENU *mp;
struct _menu_info *mi;
{
	word i, keycode, mstatus;
	byte autoexe = 0, firstrun = 0;

	if (!(mp->menu_type & MENU_NOOPEN) || !mp->menu_win) {
	   if ((mp->menu_type & MENU_TYPEMASK) == MENU_BAR) {
	      byte maxwidth;
	      for (maxwidth = 0, i = 0; i < mp->num_items; i++)
		  maxwidth += strlen(mp->item[i].text);
	      mp->menu_win = win_open(mp->menu_left, mp->menu_top,
				      mp->menu_left + (maxwidth - 1),
				      mp->menu_top,
				      CUR_NONE, CON_RAW, mp->atr_normal,
				      KEY_NONE);
	   }
	   else {
	      byte width, maxwidth;
	      for (maxwidth = 0, i = 0; i < mp->num_items; i++) {
		  width = mp->item[i].text ? strlen(mp->item[i].text) : 0;
		  if (width > maxwidth)
		     maxwidth = width;
	      }
	      mp->menu_win = win_boxopen(mp->menu_left, mp->menu_top,
					 mp->menu_left + maxwidth + 1,
					 mp->menu_top + mp->num_items + 1,
					 CUR_NONE,CON_RAW,mp->atr_normal,
					 KEY_NONE,
					 mp->menu_border, mp->menu_border,
					 mp->menu_border, mp->menu_border,
					 mp->atr_border,
					 mp->menu_title, mp->atr_title);
	   }

	   mcur_reset();
	}

	win_settop(mp->menu_win);
	for (i = 0; i < mp->num_items; i++)
	    mp->item[i].flags |= OPT_REFRESH;

#if 0
	keycode = win_keyscan();
	if (keycode) {
	   if (keycode < 0x100)
	      keycode = inl_toupper(keycode);
	   for (i = 0; i < mi->hotkeys->num_items; i++) {    /* any hotkeys? */
	       if (keycode == mi->hotkeys->item[i].key) {
		  win_keygetc();
		  mi->key      = keycode;
		  mi->flags    = mi->hotkeys->item[i].flags;
		  mi->function = mi->hotkeys->item[i].function;
		  mi->menu     = mi->hotkeys->item[i].menu;
		  if (menu_action(mp,mi) == 2)
		     goto do_shut;
	       }
	   }
	}
#endif

	if (mp->menu_type & MENU_DELAYIMMEDIATE) {
	   firstrun = 1;
	   goto loop;
	}

again:	autoexe = 1;
loop:	menu_show(mp,0);

	if (autoexe && !firstrun) {
	   autoexe = 0;
	   if (mp->item[mp->opt_last].flags & OPT_IMMEDIATE)
	      goto select;
	}

	mi->key      = 0;			       /* reset action stuff */
	mi->flags    = 0;
	mi->function = NULL;
	mi->menu     = NULL;

getkey: while (!win_keyscan()) {
	      if (!(mstatus = (mcur_buttons() & MCUR_LEFTBUTTON)))
		 mstatus = mcur_status();

	      if (mstatus & (MCUR_LEFTCLICK | MCUR_LEFTBUTTON)) {
		 if (mcur_getwin() == mp->menu_win) {
		    byte mhorpos, mverpos;

		    if (mcur_getcurpos(&mhorpos,&mverpos)) {
		       mhorpos--;
		       mverpos--;
		       if ((mp->menu_type & MENU_TYPEMASK) == MENU_BAR) {
			  byte whorpos, wverpos;

			  mcur_getwinpos(&whorpos,&wverpos);
			  whorpos--;
			  wverpos--;
			  if (wverpos == windows[mp->menu_win].cur_top) {
			     byte hor, len;

			     for (hor = 0, i = 0; i < mp->num_items; i++) {
				 if (mp->item[i].text) {
				    len = strlen(mp->item[i].text);
				    if (mp->item[i].key &&
					mhorpos >= hor && mhorpos < (hor + len)) {
				       if (i == mp->opt_last && !(mstatus & MCUR_LEFTCLICK))
					  break;
				       mp->item[mp->opt_last].flags |= OPT_REFRESH;
				       mp->item[i].flags |= OPT_REFRESH;
				       mp->opt_last = i;
				       if (mstatus & MCUR_LEFTCLICK) {
					  menu_show(mp,0);
					  goto select;
				       }
				       firstrun = 0;
				       goto again;
				    }
				    hor += len;
				 }
			     }
			  }
		       }
		       else if (mverpos < mp->num_items &&
				mp->item[mverpos].key &&
				(mverpos != mp->opt_last || (mstatus & MCUR_LEFTCLICK))) {
			  mp->item[mp->opt_last].flags |= OPT_REFRESH;
			  mp->item[mverpos].flags |= OPT_REFRESH;
			  mp->opt_last = mverpos;
			  if (mstatus & MCUR_LEFTCLICK) {
			     menu_show(mp,0);
			     goto select;
			  }
			  firstrun = 0;
			  goto again;
		       }
		    }
		    else {
		       keycode = MKEY_CURBUTTON;
		       goto hot;
		    }
		 }
		 else if (mp->menu_parent != NULL &&
			 (mp->menu_parent->menu_type & MENU_TYPEMASK) == MENU_BAR &&
			  mcur_getwin() == mp->menu_parent->menu_win) {
		    byte mhorpos, mverpos;

		    if (mcur_getcurpos(&mhorpos,&mverpos)) {
		       mhorpos--;
		       mverpos--;
		       if ((mp->menu_parent->menu_type & MENU_TYPEMASK) == MENU_BAR) {
			  byte hor, len;

			  if (mverpos == windows[mp->menu_parent->menu_win].cur_top) {
			     for (hor = 0, i = 0; i < mp->menu_parent->num_items; i++) {
				 if (mp->menu_parent->item[i].text) {
				    len = strlen(mp->menu_parent->item[i].text);
				    if (mp->menu_parent->item[i].key &&
					mhorpos >= hor && mhorpos < (hor + len)) {
				       if (i == mp->menu_parent->opt_last)
					  break;
				       mp->menu_parent->item[mp->menu_parent->opt_last].flags |= OPT_REFRESH;
				       mp->menu_parent->item[i].flags |= OPT_REFRESH;
				       mp->menu_parent->opt_last = i;
				       mi->flags = OPT_SHUTONE | OPT_IMMEDIATE;
				       goto do_action;
				    }
				    hor += len;
				 }
			     }
			  }
		       }
		       else if (mverpos < mp->menu_parent->num_items &&
				mp->menu_parent->item[mverpos].key &&
				mverpos != mp->menu_parent->opt_last) {
			  mp->menu_parent->item[mp->menu_parent->opt_last].flags |= OPT_REFRESH;
			  mp->menu_parent->item[mverpos].flags |= OPT_REFRESH;
			  mp->menu_parent->opt_last = mverpos;
			  mi->flags = OPT_SHUTONE | OPT_IMMEDIATE;
			  goto do_action;
		       }
		    }
		 }
		 else if (mstatus & MCUR_LEFTCLICK)
		    goto back;
	      }
	      else if (mstatus & MCUR_RIGHTCLICK)
		 goto back;

	      win_idle();
	}

	keycode = win_keygetc();		      /* get key/mouse input */
	if (keycode < 0x100)
	   keycode = inl_toupper(keycode);

	if ((mp->menu_type & MENU_TYPEMASK) == MENU_BAR) {
	   if (keycode == MKEY_RIGHTMOVE || keycode == Right) {
	      if (menu_next(mp,keycode == MKEY_RIGHTMOVE)) goto again;
	      else					   goto getkey;
	   }
	   if (keycode == MKEY_LEFTMOVE || keycode == Left)
	      if (menu_prev(mp,keycode == MKEY_LEFTMOVE)) goto again;
	      else					  goto getkey;
	}
	else {
	   if (mp->menu_parent != NULL &&
	       (mp->menu_parent->menu_type & MENU_TYPEMASK) == MENU_BAR &&
	       ((keycode == MKEY_LEFTMOVE || keycode == Left) &&
		menu_prev(mp->menu_parent,keycode == MKEY_LEFTMOVE)) ||
	       ((keycode == MKEY_RIGHTMOVE || keycode == Right) &&
		menu_next(mp->menu_parent,keycode == MKEY_RIGHTMOVE))) {
	      mi->flags = OPT_SHUTONE | OPT_IMMEDIATE;
	      goto do_action;
	   }

	   if (keycode == MKEY_DOWNMOVE || keycode == Down) {
	      if (menu_next(mp,keycode == MKEY_DOWNMOVE)) goto again;
	      else					  goto getkey;
	   }
	   if (keycode == MKEY_UPMOVE || keycode == Up) {
	      if (menu_prev(mp,keycode == MKEY_UPMOVE)) goto again;
	      else					goto getkey;
	   }
	}

	if (keycode == MKEY_LEFTBUTTON || keycode == Enter || keycode == ' ') {
select:    mi->key	= mp->item[mp->opt_last].key;
	   mi->flags	= (mp->item[mp->opt_last].flags & ~OPT_IMMEDIATE);
	   mi->function = mp->item[mp->opt_last].function;
	   mi->menu	= mp->item[mp->opt_last].menu;
	   firstrun = 0;
	   goto do_action;
	}

	if (keycode == MKEY_RIGHTBUTTON || keycode == Esc) {
back:	   mi->key    = keycode;
	   if	   (mp->menu_type & MENU_SHUTTOTOP) mi->flags  = OPT_SHUTTOTOP;
	   else if (mp->menu_type & MENU_SHUTALL)   mi->flags  = OPT_SHUTALL;
	   else 				    mi->flags  = OPT_SHUTONE;
	   goto do_action;
	}

	for (i = 0; i < mp->num_items; i++) {		 /* normal menu keys */
	    if (keycode == mp->item[i].key) {
	       if (i != mp->opt_last) {
		  mp->item[mp->opt_last].flags |= OPT_REFRESH;
		  mp->item[i].flags |= OPT_REFRESH;
		  mp->opt_last = i;
		  menu_show(mp,0);
	       }
	       mi->key	    = keycode;
	       mi->flags    = mp->item[i].flags;
	       mi->function = mp->item[i].function;
	       mi->menu     = mp->item[i].menu;
	       firstrun = 0;
	       goto do_action;
	    }
	}

hot:	for (i = 0; i < mi->hotkeys->num_items; i++) {	     /* any hotkeys? */
	    if (keycode == mi->hotkeys->item[i].key) {
	       mi->key	    = keycode;
	       mi->flags    = mi->hotkeys->item[i].flags;
	       mi->function = mi->hotkeys->item[i].function;
	       mi->menu     = mi->hotkeys->item[i].menu;
	       goto do_action;
	    }
	}

	goto getkey;

do_action:
	switch (menu_action(mp,mi)) {
	       case 0:
		    goto loop;
	       case 1:
		    goto again;
	       case 2:
#if 0
do_shut:
#endif
		    if (mp->menu_type & MENU_NOCLOSE) {
		       mp->item[mp->opt_last].flags |= OPT_REFRESH;
		       menu_show(mp,1);
		    }
		    else {
		       win_close(mp->menu_win);
		       mp->menu_win = 0;
		    }
		    return;
	       default:
		    break;
	}
}/*menu_new()*/


word _win_exp _win_fast win_menu(mp, hp)		  /* menu sys main entry call */
WIN_MENU *mp;
WIN_HOTKEYS *hp;
{
	struct _menu_info mi;

	if (windows == NULL)
	   return (0);

	mi.hotkeys  = hp;
	mi.flags    = OPT_NONE;
	mi.function = NULL;
	mi.menu     = mp;
	do menu_action(NULL,&mi);
	while (mi.function != NULL || mi.menu != NULL);

	return (mi.key);
}/*win_menu()*/


static void near _win_fast select_show(sp)
WIN_SELECT *sp;
{
	word i;
	byte ver, len;
	word attrib;
	SCR_PTR win_ptr;

	win_setwp(sp->select_win);
	ver = wp->cur_top;
	win_ptr = wp->win_buf + ver * wp->win_horlen + wp->cur_left;

	for (i = sp->opt_topline;
	     i < sp->num_items && ver <= wp->cur_bottom;
	     i++, ver++, win_ptr += wp->win_horlen) {

	    if (sp->item[i].flags & OPT_REFRESH) {
	       attrib = ((word) (sp->opt_last == i ?
				 sp->atr_select : sp->atr_normal)) << 8;
	       win_setbuf(win_ptr,attrib | (byte) ' ',wp->cur_horlen);
	       if (sp->init != NULL)
		  (*sp->init)(sp,i);
	       for (len = 0;
		    sp->item[i].text[len] && len < wp->cur_horlen;
		    len++)
		   win_ptr[len] = attrib | (byte) sp->item[i].text[len];
	       scr_update(wp,wp->cur_left,ver,wp->cur_horlen);
	       sp->item[i].flags &= ~OPT_REFRESH;
	    }
	}

	if (ver <= wp->cur_bottom) {
	   wp->cur_verpos = ver - wp->cur_top;
	   win_setattrib(win_idx,sp->atr_normal);
	   win_clreos(win_idx);
	}

	if ((sp->select_flags & SELECT_SCROLLBAR) && wp->cur_verlen > 2) {
	   byte newscrollbar;

	   if (!sp->opt_last)
	      newscrollbar = wp->cur_top + 1;
	   else if (sp->opt_last == (sp->num_items - 1))
	      newscrollbar = wp->cur_bottom - 1;
	   else {
	      if (sp->num_items > wp->cur_verlen)
		 newscrollbar = wp->cur_top + 1 +
				(sp->opt_last /
				 ((sp->num_items + (wp->cur_verlen - 3)) /
				  (wp->cur_verlen - 2)));
	      else
		 newscrollbar = wp->cur_top + 1 +
				(sp->opt_last *
				 (wp->cur_verlen / sp->num_items));
	      if (newscrollbar > wp->cur_bottom - 1)
		 newscrollbar = wp->cur_bottom - 1;
	   }

	   if (newscrollbar != sp->opt_scrollbar) {
	      win_ptr = wp->win_buf + wp->cur_right + 1;
	      if (sp->opt_scrollbar) {
		 win_ptr[sp->opt_scrollbar * wp->win_horlen] =
			(((word) sp->atr_scrollbar) << 8) | (byte) '\260';
		 scr_update(wp,wp->win_horlen - 1,sp->opt_scrollbar,1);
	      }
	      win_ptr[newscrollbar * wp->win_horlen] =
			(((word) sp->atr_scrollselect) << 8) | (byte) 219;
	      scr_update(wp,wp->win_horlen - 1,newscrollbar,1);
	      sp->opt_scrollbar = newscrollbar;
	   }
	}
}/*select_show()*/


word _win_exp _win_fast win_select(sp)
WIN_SELECT *sp;
{
	word i, mstatus;
	word keycode;

	if (!(sp->select_flags & SELECT_NOOPEN) || !sp->select_win) {
	   sp->select_win = win_create(sp->select_left, sp->select_top,
				       sp->select_left + sp->select_width + 1,
				       sp->select_top + sp->select_depth + 1,
				       CUR_NONE,CON_RAW,sp->atr_normal,
				       KEY_NONE);
	   win_setattrib(win_idx,sp->atr_border);
	   win_line(win_idx,1,1,1,wp->win_verlen,sp->select_border);
	   win_line(win_idx,1,1,wp->win_horlen,1,sp->select_border);
	   win_line(win_idx,wp->win_horlen,1,
			    wp->win_horlen,wp->win_verlen,sp->select_border);
	   win_line(win_idx,1,wp->win_verlen,
			    wp->win_horlen,wp->win_verlen,sp->select_border);

	   if (sp->select_title) {
	      win_setattrib(win_idx,sp->atr_title);
	      if (strlen(sp->select_title) >= (wp->win_horlen - 1))
		 sp->select_title[wp->win_horlen - 1] = '\0';
	      win_xyputs(win_idx,2,1,sp->select_title);
	   }

	   win_setrange(win_idx,2,2,wp->win_horlen - 1,wp->win_verlen - 1);
	   mcur_reset();
	}
	else
	   win_setwp(sp->select_win);

	if ((sp->select_flags & SELECT_SCROLLBAR) && wp->cur_verlen >= 3) {
	   SCR_PTR win_ptr;
	   word attrib;

	   win_ptr = wp->win_buf + wp->cur_top * wp->win_horlen +
		     wp->cur_right + 1;
	   attrib = ((word) sp->atr_scrollbar) << 8;

	   *win_ptr = attrib | (byte) 0x18;
	   scr_update(wp,wp->win_horlen - 1,wp->cur_top,1);

	   for (i = wp->cur_top + 1;
		win_ptr += wp->win_horlen, i < wp->cur_bottom; i++) {
	       *win_ptr = attrib | (byte) '\260';
	       scr_update(wp,wp->win_horlen - 1,i,1);
	   }

	   *win_ptr = attrib | (byte) 0x19;
	   scr_update(wp,wp->win_horlen - 1,i,1);

	   sp->opt_scrollbar = 0;
	}

	for (i = 0; (sp->opt_topline + i) < sp->num_items && i < wp->cur_verlen; i++)
	    sp->item[sp->opt_topline + i].flags |= OPT_REFRESH;

	win_settop(sp->select_win);

again:	select_show(sp);

	while (!win_keyscan()) {
	      if (!(mstatus = (mcur_buttons() & MCUR_LEFTBUTTON)))
		 mstatus = mcur_status();

	      if (mstatus & (MCUR_LEFTCLICK | MCUR_LEFTBUTTON)) {
		 win_setwp(sp->select_win);
		 if (mcur_getwin() == win_idx) {
		    byte mhorpos, mverpos;

		    if (mcur_getcurpos(&mhorpos,&mverpos)) {
		       mhorpos--;
		       mverpos--;
		       if ((sp->opt_topline + mverpos) < sp->num_items) {
			  if ((sp->opt_topline + mverpos) != sp->opt_last) {
			     sp->item[sp->opt_last].flags |= OPT_REFRESH;
			     sp->opt_last = sp->opt_topline + mverpos;
			     sp->item[sp->opt_last].flags |= OPT_REFRESH;
			     select_show(sp);
			  }
			  if (mstatus & MCUR_DOUBLECLICK) {
			     keycode = Enter;
			     goto do_action;
			  }
			  continue;
		       }
		    }
		    else if ((sp->select_flags & SELECT_SCROLLBAR) &&
			     wp->cur_verlen >= 3) {
		       mcur_getwinpos(&mhorpos,&mverpos);
		       if (--mhorpos == (wp->cur_right + 1)) {
			  byte left, top, right, bottom;

			  mverpos--;
			  if (mverpos == wp->cur_top) {
			     mcur_getrange(&left,&top,&right,&bottom);
			     mcur_setrange(wp->win_left + wp->cur_right + 2,
					   wp->win_top + wp->cur_top + 1,
					   wp->win_left + wp->cur_right + 2,
					   wp->win_top + wp->cur_top + 1);
			     mcur_setpos(wp->win_left + wp->cur_right + 2,
					 wp->win_top + wp->cur_top + 1);

			     while ((mstatus & MCUR_LEFTBUTTON) &&
				    !(mstatus & MCUR_LEFTCLICK)) {
				   if (sp->opt_last) {
				      sp->item[sp->opt_last].flags |= OPT_REFRESH;
				      sp->item[--sp->opt_last].flags |= OPT_REFRESH;
				      if (sp->opt_last < sp->opt_topline) {
					 sp->opt_topline--;
					 win_scrolldown(sp->select_win,1);
				      }
				      select_show(sp);
				   }
				   win_idle();
				   mstatus = mcur_status();
			     }

			     mcur_setrange(left,top,right,bottom);
			     mcur_setpos(wp->win_left + wp->cur_right + 2,
					 wp->win_top + wp->cur_top + 1);
			     goto do_up;
			  }
			  else if (mverpos == wp->cur_bottom) {
			     mcur_getrange(&left,&top,&right,&bottom);
			     mcur_setrange(wp->win_left + wp->cur_right + 2,
					   wp->win_top + wp->cur_bottom + 1,
					   wp->win_left + wp->cur_right + 2,
					   wp->win_top + wp->cur_bottom + 1);
			     mcur_setpos(wp->win_left + wp->cur_right + 2,
					 wp->win_top + wp->cur_bottom + 1);

			     while ((mstatus & MCUR_LEFTBUTTON) &&
				    !(mstatus & MCUR_LEFTCLICK)) {
				   if (sp->opt_last < (sp->num_items - 1)) {
				      sp->item[sp->opt_last].flags |= OPT_REFRESH;
				      sp->item[++sp->opt_last].flags |= OPT_REFRESH;
				      if ((sp->opt_last - sp->opt_topline) >= wp->cur_verlen) {
					 sp->opt_topline++;
					 win_scrollup(sp->select_win,1);
				      }
				      select_show(sp);
				   }
				   win_idle();
				   mstatus = mcur_status();
			     }

			     mcur_setrange(left,top,right,bottom);
			     mcur_setpos(wp->win_left + wp->cur_right + 2,
					 wp->win_top + wp->cur_bottom + 1);
			     goto do_down;
			  }
			  else if (mverpos > wp->cur_top &&
				   mverpos < wp->cur_bottom) {
			     SCR_PTR win_ptr;

			     win_ptr = wp->win_buf + wp->cur_right + 1;

			     if ((mstatus & MCUR_LEFTBUTTON) &&
				 mverpos == sp->opt_scrollbar) {
				mcur_getrange(&left,&top,&right,&bottom);
				mcur_hide();
				mcur_setrange(wp->win_left + wp->cur_right + 2,
					      wp->win_top + wp->cur_top + 2,
					      wp->win_left + wp->cur_right + 2,
					      wp->win_top + wp->cur_bottom);
				mcur_setpos(wp->win_left + wp->cur_right + 2,
					    wp->win_top + mverpos + 1);

				while (!(mstatus & MCUR_LEFTCLICK)) {
				      if (mverpos != sp->opt_scrollbar) {
					 win_ptr[sp->opt_scrollbar * wp->win_horlen] =
						(((word) sp->atr_scrollbar) << 8) | (byte) '\260';
					 scr_update(wp,wp->win_horlen - 1,sp->opt_scrollbar,1);
					 win_ptr[mverpos * wp->win_horlen] =
						(((word) sp->atr_scrollselect) << 8) | (byte) 219;
					 scr_update(wp,wp->win_horlen - 1,mverpos,1);

					 sp->opt_scrollbar = mverpos;
				      }

				      mstatus = mcur_status();
				      mcur_getwinpos(&mhorpos,&mverpos);
				      mverpos--;
				}

				mcur_setrange(left,top,right,bottom);
				mcur_setpos(wp->win_left + wp->cur_right + 2,
					    wp->win_top + sp->opt_scrollbar + 1);
				mcur_show();
			     }

			     if (mverpos == (wp->cur_top + 1)) {
				sp->opt_last = 0;
				sp->opt_topline = 0;
			     }
			     else if (mverpos == (wp->cur_bottom - 1)) {
				sp->opt_last = sp->num_items - 1;
				if (sp->num_items > wp->cur_verlen)
				   sp->opt_topline = sp->num_items - wp->cur_verlen;
				else
				   sp->opt_topline = 0;
			     }
			     else {
				if (sp->num_items > wp->cur_verlen) {
				   sp->opt_last = (mverpos - (wp->cur_top + 1)) *
						  ((sp->num_items + (wp->cur_verlen - 3)) / (wp->cur_verlen - 2));
				   if (sp->opt_last >= sp->num_items)
				      sp->opt_last = sp->num_items - 1;
				   if (sp->opt_last > (wp->cur_verlen / 2)) {
				      sp->opt_topline = sp->opt_last - (wp->cur_verlen / 2);
				      if ((sp->opt_topline + wp->cur_verlen) > sp->num_items)
					 sp->opt_topline = sp->num_items - wp->cur_verlen;
				   }
				   else
				      sp->opt_topline = 0;
				}
				else {
				   sp->opt_last = (mverpos - (wp->cur_top + 1)) /
						  (wp->cur_verlen / sp->num_items);
				   if (sp->opt_last >= sp->num_items)
				      sp->opt_last = sp->num_items - 1;
				   sp->opt_topline = 0;
				}
			     }

			     for (i = 0; (sp->opt_topline + i) < sp->num_items && i < wp->cur_verlen; i++)
				 sp->item[sp->opt_topline + i].flags |= OPT_REFRESH;

			     goto again;
			  }
			  else
			     continue;
		       }
		    }
		 }

		 keycode = MKEY_CURBUTTON;
		 goto do_action;
	      }
	      else if (mstatus & MCUR_RIGHTCLICK) {
		 keycode = Esc;
		 goto do_exit;
	      }

	      win_idle();
	}

	keycode = win_keygetc();

	win_setwp(sp->select_win);
	switch (keycode) {
	       case Up:
	       case MKEY_UPMOVE:
do_up:		    if (sp->opt_last) {
		       sp->item[sp->opt_last].flags |= OPT_REFRESH;
		       sp->item[--sp->opt_last].flags |= OPT_REFRESH;
		       if (sp->opt_last < sp->opt_topline) {
			  sp->opt_topline--;
			  win_scrolldown(sp->select_win,1);
		       }
		    }
		    break;

	       case Down:
	       case MKEY_DOWNMOVE:
do_down:	    if (sp->opt_last < (sp->num_items - 1)) {
		       sp->item[sp->opt_last].flags |= OPT_REFRESH;
		       sp->item[++sp->opt_last].flags |= OPT_REFRESH;
		       if ((sp->opt_last - sp->opt_topline) >= wp->cur_verlen) {
			  sp->opt_topline++;
			  win_scrollup(sp->select_win,1);
		       }
		    }
		    break;

	       case PgUp:
		    if (sp->opt_topline >= (wp->cur_verlen - 1)) {
		       sp->item[sp->opt_last].flags |= OPT_REFRESH;
		       sp->opt_topline -= (wp->cur_verlen - 1);
		       sp->opt_last    -= (wp->cur_verlen - 1);
		       for (i = 0; (sp->opt_topline + i) < sp->num_items && i < wp->cur_verlen; i++)
			   sp->item[sp->opt_topline + i].flags |= OPT_REFRESH;
		       break;
		    }
		    /* fallthrough - conditional */

	       case Home:
		    if (sp->opt_last) {
		       if (sp->opt_topline) {
			  sp->opt_topline = 0;
			  for (i = 0; (sp->opt_topline + i) < sp->num_items && i < wp->cur_verlen; i++)
			      sp->item[i].flags |= OPT_REFRESH;
		       }
		       else {
			  sp->item[sp->opt_last].flags |= OPT_REFRESH;
			  sp->item[0].flags |= OPT_REFRESH;
		       }
		       sp->opt_last = 0;
		    }
		    break;

	       case PgDn:
		    if ((sp->opt_topline + 2 * (wp->cur_verlen - 1)) <
			sp->num_items) {
		       sp->opt_topline += (wp->cur_verlen - 1);
		       sp->opt_last    += (wp->cur_verlen - 1);
		       for (i = 0; (sp->opt_topline + i) < sp->num_items && i < wp->cur_verlen; i++)
			   sp->item[sp->opt_topline + i].flags |= OPT_REFRESH;
		       break;
		    }
		    /* fallthrough - conditional */

	       case End:
		    if (sp->opt_last < (sp->num_items - 1)) {
		       word newtopline;

		       sp->item[sp->opt_last].flags |= OPT_REFRESH;
		       sp->opt_last = sp->num_items - 1;
		       sp->item[sp->opt_last].flags |= OPT_REFRESH;
		       newtopline = sp->num_items >= wp->cur_verlen ?
				    sp->num_items - wp->cur_verlen : 0;
		       if (sp->opt_topline != newtopline) {
			  sp->opt_topline = newtopline;
			  for (i = 0; (sp->opt_topline + i) < sp->num_items && i < wp->cur_verlen; i++)
			      sp->item[sp->opt_topline + i].flags |= OPT_REFRESH;
		       }
		    }
		    break;

	       default:
do_action:	    if (sp->action != NULL) {
		       if (!(*sp->action)(sp,keycode)) {
			  win_settop(sp->select_win);
			  break;
		       }
		    }
		    else if (keycode != MKEY_LEFTBUTTON &&
			     keycode != Enter && keycode != ' ' &&
			     keycode != MKEY_CURBUTTON)
		       break;

	       case Esc:
	       case MKEY_RIGHTBUTTON:
do_exit:	    if (!(sp->select_flags & SELECT_NOCLOSE)) {
		       win_close(sp->select_win);
		       sp->select_win = 0;
		       sp->opt_scrollbar = 0;
		    }
		    return (keycode);
	}/*switch*/

	mcur_reset();
	goto again;
}/*win_select()*/


/* end of win_menu.c */
