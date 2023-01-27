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


/* window.c - general global functions */
#include <stdio.h>
#ifdef __MSDOS__
#include <alloc.h>
#include <mem.h>
#endif
#ifdef __PROTO__
#include <stdlib.h>
#endif
#include <string.h>
#define __WINDOW_MAIN_
#include "window.h"


#define NUM_ATTRIBS 22
static struct {
	byte attrib,
	     mask,
	     value;
} attribs[NUM_ATTRIBS] = {
		{ ATR_NORMAL   , 0x00, 0x07 },	 /* all attribs off; normal. */
		{ ATR_BOLD     , 0xff, 0x08 },	 /* bold		     */
		{ ATR_UNDERLINE, 0xf8, 0x01 },	 /* underline		     */
		{ ATR_BLINK    , 0xff, 0x80 },	 /* blink		     */
		{ ATR_REVERSE  , 0xf8, 0x70 },	 /* reverse		     */
		{ ATR_INVISIBLE, 0x88, 0x00 },	 /* invisible		     */
		{ ATR_F_BLACK  , 0xf8, 0x00 },	 /* black foreground	     */
		{ ATR_F_BLUE   , 0xf8, 0x04 },	 /* red 		     */
		{ ATR_F_GREEN  , 0xf8, 0x02 },	 /* green		     */
		{ ATR_F_CYAN   , 0xf8, 0x06 },	 /* yellow		     */
		{ ATR_F_RED    , 0xf8, 0x01 },	 /* blue		     */
		{ ATR_F_MAGENTA, 0xf8, 0x05 },	 /* magenta		     */
		{ ATR_F_YELLOW , 0xf8, 0x03 },	 /* cyan		     */
		{ ATR_F_WHITE  , 0xf8, 0x07 },	 /* white		     */
		{ ATR_B_BLACK  , 0x8f, 0x00 },	 /* black background	     */
		{ ATR_B_BLUE   , 0x8f, 0x40 },	 /* red 		     */
		{ ATR_B_GREEN  , 0x8f, 0x20 },	 /* green		     */
		{ ATR_B_CYAN   , 0x8f, 0x60 },	 /* yellow		     */
		{ ATR_B_RED    , 0x8f, 0x10 },	 /* blue		     */
		{ ATR_B_MAGENTA, 0x8f, 0x50 },	 /* magenta		     */
		{ ATR_B_YELLOW , 0x8f, 0x30 },	 /* cyan		     */
		{ ATR_B_WHITE  , 0x8f, 0x70 }};  /* white		     */


#define L1	    0x01
#define L2	    0x02
#define T1	    0x04
#define T2	    0x08
#define R1	    0x10
#define R2	    0x20
#define B1	    0x40
#define B2	    0x80

byte _win_exp line_ascii[] = {
	T1|B1, L1|T1|B1, L2|T1|B1, L1|T2|B2, L1|B2, L2|B1, L2|T2|B2, T2|B2,
	L2|B2, L2|T2, L1|T2, L2|T1, L1|B1, T1|R1, L1|T1|R1, L1|R1|B1, T1|R1|B1,
	L1|R1, L2|T1|R1|B1, T1|R2|B1, T2|R1|B2, T2|R2, R2|B2, L2|T2|R2,
	L2|R2|B2, T2|R2|B2, L2|R2, L2|T2|R2|B2, L2|T1|R2, L1|T2|R1, L2|R2|B1,
	L1|R1|B2, T2|R1, T1|R2, R2|B1, R1|B2, L1|T2|R1|B2, L2|T1|R2|B1, L1|T1,
	R1|B1 };
/* " ƒÕ≥Ÿæ∫ΩºƒƒÕ¿¡œ”– ÕÕÕ‘œœ»  ≥ø∏≥¥µ∫∂π⁄¬—√≈ÿ«◊Œ’——∆ÿÿÃŒŒ∫∑ª∫∂π∫∂π÷“À«◊Œ«◊Œ…ÀÀÃŒŒÃŒŒ" */
byte _win_exp line_table[] =
" \304\315\263\331\276\272\275\274\304\304\315\300\301\317\323\320\312\315\315\315\324\317\317\310\312\312\
\263\277\270\263\264\265\272\266\271\332\302\321\303\305\330\307\327\316\325\321\321\306\330\330\314\316\316\
\272\267\273\272\266\271\272\266\271\326\322\313\307\327\316\307\327\316\311\313\313\314\316\316\314\316\316";
byte _win_exp line_ascii2[]  = { L1|R1, L2|R2, T1|B1, L1|T1|R1|B1 };
byte _win_exp line_table2[]  =
   " -=|++|++--=+-=+-====+==+==|++||||||+-=|++|+++==|++|++|++||||||+-=|++|+++==|++|++";
char _win_exp line_set2[]    = "-=|+";


int _win_exp _win_fast win_init(max_windows, curtype, conemu, chrattrib, keyemu)
int max_windows;
byte curtype, conemu, chrattrib, keyemu;
{		   /* init window system - return 1 for success, 0 for fail  */
	register int i;

	if (windows != NULL)
	   return (0);
	if (max_windows < 1 || max_windows > 256)
	   return (0);

	if ((windows = (WIN_REC *) malloc(sizeof (WIN_REC) * max_windows)) == NULL)
	   return (0);
	num_windows = max_windows;
	for (wp = windows, i = 0; i < num_windows; wp++, i++)
	    wp->win_status = WIN_FREE;

	windows->win_buf = NIL;

	if ((win_stack = (byte *) malloc(max_windows)) == NULL) {
	   free(windows);
	   windows = NULL;
	   return (0);
	}
	win_st_top = 0;
	win_stack[win_st_top] = 0;

	if (!scr_init()) {
	   if (windows->win_buf != NIL)
	      myfarfree(windows->win_buf);
	   free(windows);
	   windows = NULL;
	   free(win_stack); 
	   return (0);
	}

	if ((win_tpl = (byte *) malloc(win_maxhor * win_maxver)) == NULL) {
	   if (windows->win_buf != NIL)
	      myfarfree(windows->win_buf);
	   free(windows);
	   windows = NULL;
	   free(win_stack);
	   return (0);
	}
	memset(win_tpl,0,win_maxhor * win_maxver);

	if ((win_mouse = (WIN_MOUSE *) malloc(sizeof (WIN_MOUSE))) == NULL) {
	   if (windows->win_buf != NIL)
	      myfarfree(windows->win_buf);
	   free(windows);
	   windows = NULL;
	   free(win_stack);
	   free(win_tpl);
	   return (0);
	}
	memset(win_mouse,0,sizeof (WIN_MOUSE));

	windows->win_status = WIN_ACTIVE;
	windows->win_left   = windows->cur_left   = 0;
	windows->win_top    = windows->cur_top	  = 0;
	windows->win_right  = windows->cur_right  = win_maxhor - 1;
	windows->win_bottom = windows->cur_bottom = win_maxver - 1;
	windows->win_horlen = windows->cur_horlen = win_maxhor;
	windows->win_verlen = windows->cur_verlen = win_maxver;
	windows->win_below  = 0;
	windows->chr_attrib = chrattrib;
	windows->con_seqbuf = NULL;

	if (!win_setconemu(0,conemu)) {
	   if (windows->win_buf != NIL)
	      myfarfree(windows->win_buf);
	   free(windows);
	   windows = NULL;
	   free(win_stack); 
	   free(win_tpl);
	   free(win_mouse);
	   return (0);
	}
	win_setcur(0,curtype);
	win_setattrib(0,chrattrib);
	win_setkeyemu(0,keyemu);		   /* set keyboard emulation */
	win_setidle(win_timeslice);		 /* set window idle function */

	win_keyreset(); 		       /* reset keyboard ring buffer */

	return (1);
}/*win_init()*/


int _win_exp _win_fast win_deinit(/*void*/)		       /* deinitialize window interface */
{
	register int i;

	if (windows == NULL)		/* wasn't active in the first place! */
	   return (0);

	win_settop(0);	  /* restore win 0 (orig scr), unblank, curpos/shape */
	scr_deinit();			      /* deinit hardware - if needed */

	for (wp = windows, i = 0; i < num_windows; wp++, i++) {
	    if (wp->win_status != WIN_FREE) {		/* free all win_bufs */
	       myfarfree(wp->win_buf);
	       if (wp->con_seqbuf)
		  free(wp->con_seqbuf);
	    }
	}

	free(windows);					  /* free win arrays */
	windows = NULL;
	free(win_stack);
	free(win_tpl);
	free(win_mouse);

	return (1);
}/*win_deinit()*/


#ifdef __PROTO__
static void cdecl (*win_idlefunc) (void);
#else
static void cdecl (*win_idlefunc) ();
#endif

void _win_exp _win_fast win_idle(/*void*/)
{
	if (windows != NULL && win_idlefunc != NULL)
	   (*win_idlefunc)();
}/*win_idle()*/


int _win_exp _win_fast win_setidle(idlefunc)
#ifdef __PROTO__
void cdecl (*idlefunc)(void);
#else
void cdecl (*idlefunc)(/*void*/);
#endif
{
	if (windows == NULL)
	   return (0);

	win_idlefunc = idlefunc;
	return (1);
}/*win_setidle()*/


WIN_IDX _win_exp _win_fast win_create(winleft, wintop, winright, winbottom,
			     curtype, conemu, chrattrib, keyemu)
byte winleft, wintop, winright, winbottom;
byte curtype, conemu, chrattrib, keyemu;
{
	if (windows == NULL)
	   return (0);
	if (winleft < 1 || winright  > win_maxhor || winright  < winleft ||
	    wintop  < 1 || winbottom > win_maxver || winbottom < wintop  )
{
printf("winleft=%d winright=%d wintop=%d winbottom=%d win_maxhor=%d win_maxver=%d\n",
       winleft, winright, wintop, winbottom, win_maxhor, win_maxver);
	   return (0);
}

	for (wp = win_getwp(1), win_idx = 1;
	     win_idx < num_windows && wp->win_status != WIN_FREE;
	     wp++, win_idx++);
	if (win_idx >= num_windows)		     /* no free window found */
	   return (0);

	wp->win_horlen = (winright - winleft) + 1;
	wp->win_verlen = (winbottom - wintop) + 1;
	wp->win_buf = (SCR_PTR) myfaralloc(sizeof (SCR_CELL) * wp->win_horlen * wp->win_verlen);
	if (wp->win_buf == NIL) 		   /* can't alloc window buf */
	   return (0);

	wp->win_status = WIN_HIDDEN;	       /* make belief for cls/settop */
	wp->win_left   = winleft - 1;
	wp->win_top    = wintop - 1;
	wp->win_right  = winright - 1;
	wp->win_bottom = winbottom - 1;
	wp->win_below  = wp->win_horlen * wp->win_verlen;
	win_setcur(win_idx,curtype);
	win_setrange(win_idx,1,1,wp->win_horlen,wp->win_verlen);
	wp->con_seqbuf = NULL;
	if (!win_setconemu(win_idx,conemu)) {
	   myfarfree(wp->win_buf);
	   return (0);
	}
	win_setattrib(win_idx,chrattrib);
	win_setkeyemu(win_idx,keyemu);		   /* set keyboard emulation */
	win_cls(win_idx);				     /* clear window */

	return (win_idx);			      /* return window index */
}/*win_create()*/


/* open a window and return window index or 0 for fail */
WIN_IDX _win_exp _win_fast win_open(winleft, wintop, winright, winbottom,
			   curtype, conemu, chrattrib, keyemu)
byte winleft, wintop, winright, winbottom;
byte curtype, conemu, chrattrib, keyemu;
{
	if (!win_create(winleft,wintop,winright,winbottom,
			curtype,conemu,chrattrib,keyemu))
	   return (0);

	win_settop(win_idx);
	return (win_idx);
}/*win_open()*/


/* open a boxed window and return window index or 0 for fail */
WIN_IDX _win_exp _win_fast win_boxopen(winleft, wintop, winright, winbottom,
			      curtype, conemu, chrattrib, keyemu,
			      leftline, topline, rightline, bottomline,
			      lineattrib,
			      wintitle, titleattrib)
byte winleft, wintop, winright, winbottom;
byte curtype, conemu, chrattrib, keyemu;
byte leftline, topline, rightline, bottomline;
byte lineattrib;
char *wintitle;
byte titleattrib;
{
	if (windows == NULL ||
	    (winright - 2 < winleft || winbottom - 2 < wintop) ||
	    !win_create(winleft,wintop,winright,winbottom,
			curtype,conemu,chrattrib,keyemu))
	   return (0);

	win_setattrib(win_idx,lineattrib);
	win_line(win_idx,1,1,1,wp->win_verlen,leftline);
	win_line(win_idx,1,1,wp->win_horlen,1,topline);
	win_line(win_idx,wp->win_horlen,1,
			 wp->win_horlen,wp->win_verlen,rightline);
	win_line(win_idx,1,wp->win_verlen,
			 wp->win_horlen,wp->win_verlen,bottomline);

	if (wintitle) {
	   win_setattrib(win_idx,titleattrib);
	   if (strlen(wintitle) >= (wp->win_horlen - 1))
	      wintitle[wp->win_horlen - 1] = '\0';
	   win_xyputs(win_idx,2,1,wintitle);
	}

	win_setrange(win_idx,2,2,wp->win_horlen - 1,wp->win_verlen - 1);
	win_setattrib(win_idx,chrattrib);
	win_settop(win_idx);	   /* put new win on top of stack and screen */

	return (win_idx);			      /* return window index */
}/*win_boxopen()*/


int _win_exp _win_fast win_close(win)		  /* close specified window and dealloc win_buf */
WIN_IDX win;
{
	if (windows == NULL)
	   return (0);
	if (win < 1 || win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 	      /* window wasn't open! */
	   return (0);

	scr_unblank();

	myfarfree(wp->win_buf); 			  /* dealloc win_buf */
	if (wp->con_seqbuf)
	   free(wp->con_seqbuf);
	if (wp->win_status == WIN_ACTIVE)     /* meaning: not already hidden */
	   win_hide(win);

	wp->win_status = WIN_FREE;		      /* mark window as free */

	return (1);
}/*win_close()*/


int _win_exp _win_fast win_hide(win)		   /* hide: remove window from stack and screen */
WIN_IDX win;
{
	WIN_REC *wp2;
	WIN_IDX *tpl_ptr;
	register int i;
	register byte hor, ver;
	WIN_IDX  lastwin;
	byte lasthor, len;

	if (windows == NULL)
	   return (0);
	if (win < 1 || win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		  /* window not open */
	   return (0);

	if (wp->win_status == WIN_HIDDEN)	    /* window already hidden */
	   return (1);

	wp->win_status = WIN_HIDDEN;		    /* mark window as hidden */

	for (i = 1; win_stack[i] != win; i++);		  /* find win on stk */
	if (i < win_st_top)			    /* remove win from stack */
	   memmove(win_stack + i,win_stack + i + 1,win_st_top - i);
	win_st_top--;			       /* and move back top of stack */

	lastwin = 0;
	lasthor = 0;
	for (ver = wp->win_top;
	     ver <= wp->win_bottom &&
	      (wp->win_below < wp->win_horlen * wp->win_verlen);
	     ver++) {

	    len = 0;
	    for (hor = wp->win_left, tpl_ptr = win_tpl + ver * win_maxhor + hor;
		 hor <= wp->win_right &&
		  (wp->win_below < wp->win_horlen * wp->win_verlen);
		 tpl_ptr++, hor++) {

		if (*tpl_ptr != win) {	   /* searching for chars of old win */
		   if (len) {
		      scr_update(win_getwp(lastwin),
				 lasthor - windows[lastwin].win_left,
				 ver - windows[lastwin].win_top,
				 len);
		      len = 0;
		   }
		   continue;
		}

		wp->win_below++;	   /* another char going under cover */
		for (i = win_st_top; i >= 0; i--) {	 /* gotcha - replace */
		    wp2 = win_getwp(win_stack[i]);	/* check this window */
		    if (wp2->win_below &&
			wp2->win_top  <= ver && wp2->win_bottom >= ver &&
			wp2->win_left <= hor && wp2->win_right	>= hor) {
		       wp2->win_below--;
		       *tpl_ptr = win_stack[i];      /* got char on this pos */
		       if (len && lastwin != *tpl_ptr) {	/* yep, mark */
			  scr_update(win_getwp(lastwin),
				     lasthor - windows[lastwin].win_left,
				     ver - windows[lastwin].win_top,
				     len);
			  len = 0;
		       }
		       if (!len) {		       /* add to refresh buf */
			  lastwin = *tpl_ptr;
			  lasthor = hor;
		       }
		       len++;
		       break;
		    }/*if*/
		}/*for stack*/
	    }/*for hor*/
	    if (len)				/* refresh underlying screen */
	       scr_update(win_getwp(lastwin),
			  lasthor - windows[lastwin].win_left,
			  ver - windows[lastwin].win_top,
			  len);
	}/*for ver*/

	scr_unblank();
	scr_setpos();			       /* position cursor in top win */
	scr_setcur();

	return (1);
}/*win_hide()*/


int _win_exp _win_fast win_settop(win)		       /* put window on top of stack and screen */
WIN_IDX win;
{			      /* for getting win to foreground or after hide */
	WIN_IDX *tpl_ptr;
	register int i;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);

	if (win == 0) { 	  /* special trick to hide all wins except 0 */
	   if (win_st_top) {			/* win 0 not only one on scr */
	      win_st_top = 0;		/* move stack top back to only win 0 */
	      if (wp->win_below == win_maxhor * win_maxver) {
		 scr_update(wp,0,0,win_maxhor * win_maxver);
		 wp->win_below = 0;	  /* win 0 is now on top all visible */
	      }
	      else {	       /* not all, but some chars of win 0 are below */
		 register word curofs, len;
		 word lastofs;

		 for (len = lastofs = curofs = 0;
		      (curofs < win_maxhor * win_maxver) && wp->win_below;
		      curofs++) {
		     if (win_tpl[curofs]) {
			wp->win_below--;
			if (!len)
			   lastofs = curofs;
			len++;
		     }
		     else if (len) {
			scr_update(wp,lastofs % win_maxhor,lastofs / win_maxhor,len);
			len = 0;
		     }
		 }/*for*/

		 if (len)
		    scr_update(wp,lastofs % win_maxhor,lastofs / win_maxhor,len);
	      }/*if*/

	      memset(win_tpl,0,win_maxhor * win_maxver);   /* set all tpl to win 0 */
	      for (wp = win_getwp(1), win_idx = 1;
		   win_idx < num_windows; wp++, win_idx++) {
		  if (wp->win_status == WIN_ACTIVE) {
		     wp->win_status = WIN_HIDDEN;
		     wp->win_below = wp->win_horlen * wp->win_verlen;
		  }
	      }
	   }

	   scr_unblank();
	   scr_setpos();
	   scr_setcur();

	   return (1);
	}

	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	if (win_gettop() == win) {		     /* we're already on top */
	   scr_unblank();
	   return (1);
	}

	if (wp->win_status == WIN_ACTIVE) {	 /* are we already on screen */
	   WIN_IDX *tpl_ptr;
	   register byte hor, ver;
	   byte lasthor, len;

	   for (i = 1; win_stack[i] != win; i++);	  /* find win on stk */
	   memmove(win_stack + i,win_stack + i + 1,win_st_top - i);
	   win_stack[win_st_top] = win; 	   /* and put him on top now */
	     /* if win_below is already 0, the first for loop will quit fast */

	     /* and now scan template to only print chars not already on top */
	   lasthor = 0;

	   for (ver = wp->win_top;
		ver <= wp->win_bottom && wp->win_below;
		ver++) {
	       len = 0;

	       for (hor = wp->win_left, tpl_ptr = win_tpl + ver * win_maxhor + hor;
		    hor <= wp->win_right && wp->win_below;
		    tpl_ptr++, hor++) {

		   if (*tpl_ptr == win) {    /* this char is already visible */
		      if (len) {
			 scr_update(wp, lasthor - wp->win_left,
				    ver - wp->win_top, len);
			 len = 0;
		      }
		   }

		   else {		 /* let's make this char visible now */
		      win_getwp(*tpl_ptr)->win_below++;
		      *tpl_ptr = win;
		      wp->win_below--;
		      if (!len) 		       /* add to refresh buf */
			 lasthor = hor;
		      len++;
		    }
	       }/*for hor*/

	       if (len) 		   /* end of this line - refresh now */
		  scr_update(wp, lasthor - wp->win_left,
			     ver - wp->win_top, len);
	   }/*for ver*/
	}

	else {				  /* in case window is marked hidden */
	   WIN_IDX *tpl_ofs;

	   wp->win_status = WIN_ACTIVE; 		    /* unhide window */
	   wp->win_below = 0;		   /* win's all on top of things now */
	   win_stack[++win_st_top] = win;      /* let's put him on top then! */
	   tpl_ptr = win_tpl + wp->win_top * win_maxhor + wp->win_left;

	   if (wp->win_horlen == win_maxhor) {	 /* update entire screen-width? */
	      tpl_ofs = tpl_ptr + win_maxhor * wp->win_verlen;
	      while (--tpl_ofs >= tpl_ptr)
		    win_getwp(*tpl_ofs)->win_below++;
	      memset(tpl_ptr,win,win_maxhor * wp->win_verlen);
	      scr_update(wp,0,0,win_maxhor * wp->win_verlen);
	   }
	   else {				      /* update line by line */
	       for (i = 0; i < wp->win_verlen; tpl_ptr += win_maxhor, i++) {
		   tpl_ofs = tpl_ptr + wp->win_horlen;
		   while (--tpl_ofs >= tpl_ptr)
			 win_getwp(*tpl_ofs)->win_below++;
		   memset(tpl_ptr,win,wp->win_horlen);
		   scr_update(wp,0,i,wp->win_horlen);
	       }
	   }
	}

	scr_unblank();
	scr_setpos();
	scr_setcur();

	return (1);
}/*win_settop()*/


int _win_exp _win_fast win_setconemu(win, emu)			 /* set window's term emulation */
WIN_IDX win;
byte emu;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	switch (emu & CON_EMUMASK) {			 /* need a seq buf ? */
	       case CON_ANSI:
	       case CON_VT52:
	       case CON_AVATAR:
		    if (!wp->con_seqbuf) {		/* ye, malloc it    */
		       wp->con_seqbuf = (byte *) malloc(MAX_SEQ);
		       if (wp->con_seqbuf == NULL)	 /* can't, fail init */
			  return (0);
		    }
		    wp->con_seqlen = 0; 		 /* ok, set len to 0 */
		    break;

	       default:
		    if (wp->con_seqbuf) {		 /* free old seq buf */
		       free(wp->con_seqbuf);
		       wp->con_seqbuf = NULL;
		    }
		    break;
	}

	wp->con_emu = emu;				 /* set emu type now */
	wp->con_horsav = wp->cur_horpos;		 /* reinit saved pos */
	wp->con_versav = wp->cur_verpos;
	if ((emu & CON_EMUMASK) == CON_AVATAR)
	   win_setattrib(win,CHR_F_CYAN);

	return (1);					 /* return succesful */
}/*win_setconemu()*/


byte _win_exp _win_fast win_getconemu(win)		     /* get terminal emulation settings */
WIN_IDX win;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	return (wp->con_emu);
}/*win_getconemu()*/


int _win_exp _win_fast win_setkeyemu(win, keyemu)		 /* set keyboard emulation type */
WIN_IDX win;
byte keyemu;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	wp->key_emu = keyemu;

	return (1);
}/*win_setkeyemu()*/


byte _win_exp _win_fast win_getkeyemu(win)			 /* get keyboard emulation type */
WIN_IDX win;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	return (wp->key_emu);
}/*win_getkeyemu()*/


int _win_exp _win_fast win_setcur(win, curtype) 		    /* set window's cursor type */
WIN_IDX win;
byte curtype;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	wp->cur_type = curtype;
	if (win_gettop() == win)	   /* only if this is the top window */
	   scr_setcur();			/* set cursor type on screen */

	return (1);					 /* return succesful */
}/*win_setcur()*/


byte _win_exp _win_fast win_getcur(win) 		    /* get window's cursor type setting */
WIN_IDX win;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	return (wp->cur_type);
}/*win_getcur()*/


int _win_exp _win_fast win_setrange(win, left, top, right, bottom)
WIN_IDX win;
byte left, top, right, bottom;
{					   /* set cursor range within window */
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	if (!left || !top || !right || !bottom)
	   return (0);

	if (left > right || top > bottom ||		   /* invalid ranges */
	    right > wp->win_horlen || bottom > wp->win_verlen)
	   return (0);

	wp->cur_left   = left - 1;
	wp->cur_top    = top - 1;
	wp->cur_right  = right	- 1;
	wp->cur_bottom = bottom - 1;
	wp->cur_horlen = (right - left) + 1;
	wp->cur_verlen = (bottom - top) + 1;
	wp->cur_horpos = wp->con_horsav = 0;
	wp->cur_verpos = wp->con_versav = 0;

	if (win_gettop() == win)	   /* only if this is the top window */
	   scr_setpos();			    /* move cursor on screen */

	return (1);
}/*win_setrange()*/


int _win_exp _win_fast win_getrange(win, left, top, right, bottom)
WIN_IDX win;
byte *left, *top, *right, *bottom;
{					   /* get cursor range within window */
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	*left	= wp->cur_left + 1;
	*top	= wp->cur_top + 1;
	*right	= wp->cur_right + 1;
	*bottom = wp->cur_bottom + 1;

	return (1);
}/*win_getrange()*/


int _win_exp _win_fast win_cls(win)		    /* clear window and move cursor to top left */
WIN_IDX win;
{				    /* ever seen such a simple function? ;-) */
	return (win_setpos(win,1,1) ? win_clreos(win) : 0);
}/*win_cls()*/


int _win_exp _win_fast win_clrbos(win)		 /* clear from beginning of win to current line */
WIN_IDX win;
{
	register byte hor, ver;
	word c;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	c = ((word) wp->chr_attrib << 8) | (byte) ' '; /* set space fillchar */

	if (wp->cur_horlen == wp->win_horlen)	/* clr entire window width ? */
	   win_setbuf(wp->win_buf + wp->cur_top * wp->win_horlen,
		      c, (wp->cur_verpos + 1) * wp->win_horlen);
	else					   /* nope, clr line by line */
	   for (ver = 0; ver <= wp->cur_verpos; ver++)
	       win_setbuf(wp->win_buf + (wp->cur_top + ver) * wp->win_horlen + wp->cur_left,
			  c, wp->cur_horlen);

	if (wp->win_status == WIN_HIDDEN)		/* that's easy stuff */
	   return (1);

	if (!wp->win_below) {				 /* topwin is easier */
	   if (wp->cur_horlen == win_maxhor)	 /* update entire screen width? */
	      scr_update(wp,0,wp->cur_top,(wp->cur_verpos + 1) * win_maxhor);
	   else {				      /* update line by line */
	      for (ver = 0; ver <= wp->cur_verpos; ver++)
		  scr_update(wp,wp->cur_left,wp->cur_top + ver,wp->cur_horlen);
	   }
	}

	else {					       /* not the top window */
	   WIN_IDX *tpl_ptr;
	   byte lasthor, len;

	   tpl_ptr = win_tpl +
		     (wp->win_top + wp->cur_top) * win_maxhor +
		     wp->win_left;
	   lasthor = 0;

	   for (ver = 0; ver <= wp->cur_verpos; tpl_ptr += win_maxhor, ver++) {
	       for (len = 0, hor = wp->cur_left;	  /* scan every line */
		    hor <= wp->cur_right;
		    hor++) {
		   if (*(tpl_ptr + hor) == win) {	  /* is pos visible? */
		      if (!len) 		       /* add to refresh buf */
			 lasthor = hor;
		      len++;
		   }

		   else if (len) {
		      scr_update(wp,lasthor,wp->cur_top + ver,len);
		      len = 0;
		   }
	       }/*forhor*/

	       if (len) 		   /* end of this line - refresh now */
		  scr_update(wp,lasthor,wp->cur_top + ver,len);
	   }/*forver*/
	}

	if (wp->con_emu & CON_UNBLANK)
	   scr_unblank();

	return (1);
}/*win_clrbos()*/


int _win_exp _win_fast win_clreos(win)		/* clear rest of win from current line downward */
WIN_IDX win;
{
	register byte hor, ver;
	word c;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	c = ((word) wp->chr_attrib << 8) | (byte) ' '; /* set space fillchar */

	if (wp->cur_horlen == wp->win_horlen)	/* clr entire window width ? */
	   win_setbuf(wp->win_buf + (wp->cur_top + wp->cur_verpos) * wp->win_horlen,
		      c, (wp->cur_verlen - wp->cur_verpos) * wp->win_horlen);
	else					   /* nope, clr line by line */
	   for (ver = wp->cur_top + wp->cur_verpos; ver <= wp->cur_bottom; ver++)
	       win_setbuf(wp->win_buf + ver * wp->win_horlen + wp->cur_left,
			  c, wp->cur_horlen);

	if (wp->win_status == WIN_HIDDEN)		/* that's easy stuff */
	   return (1);

	if (!wp->win_below) {				 /* topwin is easier */
	   if (wp->cur_horlen == win_maxhor)	 /* update entire screen width? */
	      scr_update(wp,0,wp->cur_top + wp->cur_verpos,
			 (wp->cur_verlen - wp->cur_verpos) * win_maxhor);
	   else {				      /* update line by line */
	      for (ver = wp->cur_top + wp->cur_verpos; ver <= wp->cur_bottom; ver++)
		  scr_update(wp,wp->cur_left,ver,wp->cur_horlen);
	   }
	}

	else {					       /* not the top window */
	   WIN_IDX *tpl_ptr;
	   byte lasthor, len;

	   tpl_ptr = win_tpl +
		     (wp->win_top + wp->cur_top + wp->cur_verpos) * win_maxhor +
		     wp->win_left;
	   lasthor = 0;

	   for (ver = wp->cur_top + wp->cur_verpos;
		ver <= wp->cur_bottom;
		tpl_ptr += win_maxhor, ver++) {

	       for (len = 0, hor = wp->cur_left;	  /* scan every line */
		    hor <= wp->cur_right;
		    hor++) {
		   if (*(tpl_ptr + hor) == win) {	  /* is pos visible? */
		      if (!len) 		       /* add to refresh buf */
			 lasthor = hor;
		      len++;
		   }

		   else if (len) {
		      scr_update(wp,lasthor,ver,len);
		      len = 0;
		   }
	       }/*forhor*/

	       if (len) 		   /* end of this line - refresh now */
		  scr_update(wp,lasthor,ver,len);
	   }/*forver*/
	}

	if (wp->con_emu & CON_UNBLANK)
	   scr_unblank();

	return (1);
}/*win_clreos()*/


int _win_exp _win_fast win_clrbol(win)		 /* clear from beginning of line to current pos */
WIN_IDX win;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	win_setbuf(wp->win_buf + (wp->cur_top + wp->cur_verpos) * wp->win_horlen +
				 wp->cur_left,
		   ((word) wp->chr_attrib << 8) | (byte) ' ',
		   wp->cur_horpos + 1);

	if (wp->win_status == WIN_HIDDEN)		/* that's easy stuff */
	   return (1);

	if (!wp->win_below)				 /* topwin is easier */
	   scr_update(wp,wp->cur_left,wp->cur_top + wp->cur_verpos,wp->cur_horpos + 1);

	else {					       /* not the top window */
	   WIN_IDX *tpl_ptr;
	   register byte hor, lasthor, len;

	   tpl_ptr = win_tpl +
		     (wp->win_top + wp->cur_top + wp->cur_verpos) * win_maxhor +
		     wp->win_left + wp->cur_left;

	   for (len = lasthor = 0, hor = 0;
		hor <= wp->cur_horpos;
		tpl_ptr++, hor++) {
	       if (*tpl_ptr == win) {			  /* is pos visible? */
		  if (!len)			       /* add to refresh buf */
		     lasthor = hor;
		  len++;
	       }

	       else if (len) {
		  scr_update(wp,wp->cur_left + lasthor,wp->cur_top + wp->cur_verpos,len);
		  len = 0;
	       }
	   }/*forhor*/

	   if (len)				/* end of line - refresh now */
	      scr_update(wp,wp->cur_left + lasthor,wp->cur_top + wp->cur_verpos,len);
	}

	if (wp->con_emu & CON_UNBLANK)
	   scr_unblank();

	return (1);
}/*win_clrbol()*/


int _win_exp _win_fast win_clreol(win)			 /* clear rest of line from current pos */
WIN_IDX win;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	win_setbuf(wp->win_buf + (wp->cur_top + wp->cur_verpos) * wp->win_horlen +
				 wp->cur_left + wp->cur_horpos,
		   ((word) wp->chr_attrib << 8) | (byte) ' ',
		   wp->cur_horlen - wp->cur_horpos);

	if (wp->win_status == WIN_HIDDEN)		/* that's easy stuff */
	   return (1);

	if (!wp->win_below) {				 /* topwin is easier */
	   scr_update(wp,wp->cur_left + wp->cur_horpos,
		      wp->cur_top + wp->cur_verpos,
		      wp->cur_horlen - wp->cur_horpos);
	}

	else {					       /* not the top window */
	   WIN_IDX *tpl_ptr;
	   register byte hor, lasthor, len;

	   tpl_ptr = win_tpl +
		     (wp->win_top + wp->cur_top + wp->cur_verpos) * win_maxhor +
		     wp->win_left + wp->cur_left + wp->cur_horpos;

	   for (len = lasthor = 0, hor = wp->cur_left + wp->cur_horpos;
		hor <= wp->cur_right;
		tpl_ptr++, hor++) {
	       if (*tpl_ptr == win) {			  /* is pos visible? */
		  if (!len)			       /* add to refresh buf */
		     lasthor = hor;
		  len++;
	       }

	       else if (len) {
		  scr_update(wp,lasthor,wp->cur_top + wp->cur_verpos,len);
		  len = 0;
	       }
	   }/*forhor*/

	   if (len)				/* end of line - refresh now */
	      scr_update(wp,lasthor,wp->cur_top + wp->cur_verpos,len);
	}

	if (wp->con_emu & CON_UNBLANK)
	   scr_unblank();

	return (1);
}/*win_clreol()*/


#if 0
int _win_exp _win_fast win_scrollup(win, lines) 		/* scroll up win and clear rest */
WIN_IDX win;
byte lines;
{
	register byte ver;
	byte horsav, versav, cursav;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	win_getpos(win,&horsav,&versav);
	cursav = win_getcur(win);
	win_setcur(win,CUR_HIDDEN);

	if (!lines || lines >= wp->cur_verlen)
	   lines = wp->cur_verlen;
	else {
	   if (wp->win_horlen == wp->cur_horlen) /* scroll entire win-width? */
	      win_movebuf(wp->win_buf + wp->cur_top * wp->win_horlen,
			  wp->win_buf + (wp->cur_top + lines) * wp->win_horlen,
			  (wp->cur_verlen - lines) * wp->win_horlen);

	   else 				   /* scroll up line by line */
	      for (ver = wp->cur_top; ver <= wp->cur_bottom - lines; ver++)
		  win_movebuf(wp->win_buf + ver * wp->win_horlen +
					    wp->cur_left,
			      wp->win_buf + (ver + lines) * wp->win_horlen +
					    wp->cur_left,
			      wp->cur_horlen);

	   if (!wp->win_below) {		   /* nothing below? easier! */
	      if (wp->cur_horlen == win_maxhor)       /* move entire scr-width? */
		 scr_move(0, wp->win_top + wp->cur_top,
			  0, wp->win_top + wp->cur_top + lines,
			  win_maxhor * (wp->cur_verlen - lines));
	      else					/* move line by line */
		 for (ver = wp->win_top + wp->cur_top;
		      ver <= (wp->win_top + wp->cur_bottom) - lines;
		      ver++)
		     scr_move(wp->win_left + wp->cur_left,ver,
			      wp->win_left + wp->cur_left,ver + lines,
			      wp->cur_horlen);
	   }
	   else if (wp->win_below < wp->win_horlen * wp->win_verlen) {
	      WIN_IDX *tpl_ptr;
	      register byte hor;
	      byte from, lasthor, len;

	      tpl_ptr = win_tpl + (wp->win_top + wp->cur_top) * win_maxhor +
				  wp->win_left;
	      lasthor = from = 0;

	      for (ver = wp->cur_top;
		   ver <= wp->cur_bottom - lines;
		   tpl_ptr += win_maxhor, ver++) {

		  for (len = 0, hor = wp->cur_left;    /* scan every line */
		       hor <= wp->cur_right;
		       hor++) {
		      if (*(tpl_ptr + hor) == win) {	      /* is visible? */
			 if (*(tpl_ptr + lines * win_maxhor + hor) == win) {
			    if (len) {
			       if (!from) {
				  scr_update(wp,lasthor,ver,len);
				  from = 1;
				  len = 0;
				  lasthor = hor;
			       }
			    }
			    else {
			       from = 1;
			       lasthor = hor;
			    }
			    len++;
			    continue;
			 }

			 if (len) {
			    if (from) {
			       scr_move(wp->win_left + lasthor,
					wp->win_top  + ver,
					wp->win_left + lasthor,
					wp->win_top  + ver + 1,
					len);
			       from = 0;
			       len = 0;
			       lasthor = hor;
			    }
			 }
			 else {
			    from = 0;
			    lasthor = hor;
			 }
			 len++;
			 continue;
		      }

		      if (len) {
			 if (from) scr_move(wp->win_left + lasthor,
					    wp->win_top  + ver,
					    wp->win_left + lasthor,
					    wp->win_top  + ver + 1,
					    len);
			 else	   scr_update(wp,lasthor,ver,len);
			 len = 0;
		      }
		  }/*forhor*/

		  if (len) {		   /* end of this line - refresh now */
		     if (from) scr_move(wp->win_left + lasthor,
					wp->win_top  + ver,
					wp->win_left + lasthor,
					wp->win_top  + ver + 1,
					len);
		     else      scr_update(wp,lasthor,ver,len);
		  }
	      }/*forver*/
	   }/*if*/
	}/*if*/

	win_setpos(win,1,(wp->cur_verlen - lines) + 1);
	win_clreos(win);
	win_setpos(win,horsav,versav);
	win_setcur(win,cursav);

	return (1);
}/*win_scrollup()*/


int _win_exp _win_fast win_scrolldown(win, lines)		/* scroll down window and clear */
WIN_IDX win;
byte lines;
{
	register byte ver;
	byte horsav, versav, cursav;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	win_getpos(win,&horsav,&versav);
	cursav = win_getcur(win);
	win_setcur(win,CUR_HIDDEN);

	if (!lines || lines >= wp->cur_verlen)
	   lines = wp->cur_verlen;
	else {
	   if (wp->win_horlen == wp->cur_horlen) /* scroll entire win-width? */
	      win_movebuf(wp->win_buf + (wp->cur_top + lines) * wp->win_horlen,
			  wp->win_buf + wp->cur_top * wp->win_horlen,
			  (wp->cur_verlen - lines) * wp->win_horlen);

	   else 				   /* scroll up line by line */
	      for (ver = wp->cur_bottom; ver >= wp->cur_top + lines; ver--)
		  win_movebuf(wp->win_buf + ver * wp->win_horlen +
					    wp->cur_left,
			      wp->win_buf + (ver - lines) * wp->win_horlen +
					    wp->cur_left,
			      wp->cur_horlen);

	   if (!wp->win_below) {		   /* nothing below? easier! */
	      if (wp->cur_horlen == win_maxhor)       /* move entire scr-width? */
		 scr_move(0, wp->win_top + wp->cur_top + lines,
			  0, wp->win_top + wp->cur_top,
			  win_maxhor * (wp->cur_verlen - lines));
	      else					/* move line by line */
		 for (ver = wp->win_top + wp->cur_bottom;
		      ver >= wp->win_top + wp->cur_top + lines;
		      ver--)
		     scr_move(wp->win_left + wp->cur_left,ver,
			      wp->win_left + wp->cur_left,ver - lines,
			      wp->cur_horlen);
	   }
	   else if (wp->win_below < wp->win_horlen * wp->win_verlen) {
	      WIN_IDX *tpl_ptr;
	      register byte hor;
	      byte from, lasthor, len;

	      tpl_ptr = win_tpl + (wp->win_top + wp->cur_bottom) * win_maxhor +
				  wp->win_left;
	      lasthor = from = 0;

	      for (ver = wp->cur_bottom;
		   ver >= wp->cur_top + lines;
		   tpl_ptr -= win_maxhor, ver++) {

		  for (len = 0, hor = wp->cur_left;	  /* scan every line */
		       hor <= wp->cur_right;
		       hor++) {
		      if (*(tpl_ptr + hor) == win) {	      /* is visible? */
			 if (*((tpl_ptr - (lines * win_maxhor)) + hor) == win) {
			    if (len) {
			       if (!from) {
				  scr_update(wp,lasthor,ver,len);
				  from = 1;
				  len = 0;
				  lasthor = hor;
			       }
			    }
			    else {
			       from = 1;
			       lasthor = hor;
			    }
			    len++;
			    continue;
			 }

			 if (len) {
			    if (from) {
			       scr_move(wp->win_left + lasthor,
					wp->win_top  + ver,
					wp->win_left + lasthor,
					wp->win_top  + ver + 1,
					len);
			       from = 0;
			       len = 0;
			       lasthor = hor;
			    }
			 }
			 else {
			    from = 0;
			    lasthor = hor;
			 }
			 len++;
			 continue;
		      }

		      if (len) {
			 if (from) scr_move(wp->win_left + lasthor,
					    wp->win_top  + ver,
					    wp->win_left + lasthor,
					    wp->win_top  + ver + 1,
					    len);
			 else	   scr_update(wp,lasthor,ver,len);
			 len = 0;
		      }
		  }/*forhor*/

		  if (len) {		   /* end of this line - refresh now */
		     if (from) scr_move(wp->win_left + lasthor,
					wp->win_top  + ver,
					wp->win_left + lasthor,
					wp->win_top  + ver + 1,
					len);
		     else      scr_update(wp,lasthor,ver,len);
		  }
	      }/*forver*/
	   }/*if*/
	}/*if*/

	if (lines == wp->cur_verlen)
	   win_cls(win);
	else {
	   for (ver = 1; ver <= lines; ver++) {
	       win_setpos(win,1,ver);
	       win_clreol(win);
	   }
	}

	win_setpos(win,horsav,versav);
	win_setcur(win,cursav);

	return (1);
}/*win_scrolldown()*/
#endif


int _win_exp _win_fast win_scrollup(win, lines)       /* scroll up win and clear rest */
WIN_IDX win;
byte lines;
{
	register byte ver;
	byte horsav, versav, cursav;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	win_getpos(win,&horsav,&versav);
	cursav = win_getcur(win);
	win_setcur(win,CUR_HIDDEN);

	if (!lines || lines >= wp->cur_verlen)
	   win_cls(win);
	else {
	   if (wp->win_horlen == wp->cur_horlen) /* scroll entire win-width? */
	      win_movebuf(wp->win_buf + wp->cur_top * wp->win_horlen,
			  wp->win_buf + (wp->cur_top + lines) * wp->win_horlen,
			  (wp->cur_verlen - lines) * wp->win_horlen);

	   else 				   /* scroll up line by line */
	      for (ver = wp->cur_top; ver <= wp->cur_bottom - lines; ver++)
		  win_movebuf(wp->win_buf + ver * wp->win_horlen +
					    wp->cur_left,
			      wp->win_buf + (ver + lines) * wp->win_horlen +
					    wp->cur_left,
			      wp->cur_horlen);

	   if (!wp->win_below) {		   /* nothing below? easier! */
	      scr_scrollup(wp->win_left + wp->cur_left, wp->win_top + wp->cur_top,
			   wp->cur_horlen, wp->cur_verlen,
			   lines, wp->chr_attrib);
	   }
	   else if (wp->win_below < wp->win_horlen * wp->win_verlen) {
	      WIN_IDX *tpl_ptr;
	      register byte hor;
	      byte from, lasthor, len;

	      tpl_ptr = win_tpl + (wp->win_top + wp->cur_top) * win_maxhor +
				  wp->win_left;
	      lasthor = from = 0;

	      for (ver = wp->cur_top;
		   ver <= wp->cur_bottom - lines;
		   tpl_ptr += win_maxhor, ver++) {

		  for (len = 0, hor = wp->cur_left;	  /* scan every line */
		       hor <= wp->cur_right;
		       hor++) {
		      if (*(tpl_ptr + hor) == win) {	      /* is visible? */
			 if (*(tpl_ptr + lines * win_maxhor + hor) == win) {
			    if (len) {
			       if (!from) {
				  scr_update(wp,lasthor,ver,len);
				  from = 1;
				  len = 0;
				  lasthor = hor;
			       }
			    }
			    else {
			       from = 1;
			       lasthor = hor;
			    }
			    len++;
			    continue;
			 }

			 if (len) {
			    if (from) {
			       scr_scrollup(wp->win_left + lasthor,
					    wp->win_top + ver,
					    len, 2,
					    1, wp->chr_attrib);
			       from = 0;
			       len = 0;
			       lasthor = hor;
			    }
			 }
			 else {
			    from = 0;
			    lasthor = hor;
			 }
			 len++;
			 continue;
		      }

		      if (len) {
			 if (from) scr_scrollup(wp->win_left + lasthor,
						wp->win_top + ver,
						len, 2,
						1, wp->chr_attrib);
			 else	   scr_update(wp,lasthor,ver,len);
			 len = 0;
		      }
		  }/*forhor*/

		  if (len) {		   /* end of this line - refresh now */
		     if (from) scr_scrollup(wp->win_left + lasthor,
					    wp->win_top + ver,
					    len, 2,
					    1, wp->chr_attrib);
		     else      scr_update(wp,lasthor,ver,len);
		  }
	      }/*forver*/
	   }/*if*/

	   win_setpos(win,1,(wp->cur_verlen - lines) + 1);
	   win_clreos(win);
	}/*if*/

	win_setpos(win,horsav,versav);
	win_setcur(win,cursav);

	return (1);
}/*win_scrollup()*/


int _win_exp _win_fast win_scrolldown(win, lines)     /* scroll down window and clear */
WIN_IDX win;
byte lines;
{
	register byte ver;
	byte horsav, versav, cursav;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	win_getpos(win,&horsav,&versav);
	cursav = win_getcur(win);
	win_setcur(win,CUR_HIDDEN);

	if (!lines || lines >= wp->cur_verlen)
	   win_cls(win);
	else {
	   if (wp->win_horlen == wp->cur_horlen) /* scroll entire win-width? */
	      win_movebuf(wp->win_buf + (wp->cur_top + lines) * wp->win_horlen,
			  wp->win_buf + wp->cur_top * wp->win_horlen,
			  (wp->cur_verlen - lines) * wp->win_horlen);

	   else 				   /* scroll up line by line */
	      for (ver = wp->cur_bottom; ver >= wp->cur_top + lines; ver--)
		  win_movebuf(wp->win_buf + ver * wp->win_horlen +
					    wp->cur_left,
			      wp->win_buf + (ver - lines) * wp->win_horlen +
					    wp->cur_left,
			      wp->cur_horlen);

	   if (!wp->win_below) {		   /* nothing below? easier! */
	      scr_scrolldown(wp->win_left + wp->cur_left, wp->win_top + wp->cur_top,
			     wp->cur_horlen, wp->cur_verlen,
			     lines, wp->chr_attrib);
	   }
	   else if (wp->win_below < wp->win_horlen * wp->win_verlen) {
	      WIN_IDX *tpl_ptr;
	      register byte hor;
	      byte from, lasthor, len;

	      tpl_ptr = win_tpl + (wp->win_top + wp->cur_bottom) * win_maxhor +
				  wp->win_left;
	      lasthor = from = 0;

	      for (ver = wp->cur_bottom;
		   ver >= wp->cur_top + lines;
		   tpl_ptr -= win_maxhor, ver++) {

		  for (len = 0, hor = wp->cur_left;	  /* scan every line */
		       hor <= wp->cur_right;
		       hor++) {
		      if (*(tpl_ptr + hor) == win) {	      /* is visible? */
			 if (*((tpl_ptr - (lines * win_maxhor)) + hor) == win) {
			    if (len) {
			       if (!from) {
				  scr_update(wp,lasthor,ver,len);
				  from = 1;
				  len = 0;
				  lasthor = hor;
			       }
			    }
			    else {
			       from = 1;
			       lasthor = hor;
			    }
			    len++;
			    continue;
			 }

			 if (len) {
			    if (from) {
			       scr_scrolldown(wp->win_left + lasthor,
					      wp->win_top + ver,
					      len, 2,
					      1, wp->chr_attrib);
			       from = 0;
			       len = 0;
			       lasthor = hor;
			    }
			 }
			 else {
			    from = 0;
			    lasthor = hor;
			 }
			 len++;
			 continue;
		      }

		      if (len) {
			 if (from) scr_scrolldown(wp->win_left + lasthor,
						  wp->win_top + ver,
						  len, 2,
						  1, wp->chr_attrib);
			 else	   scr_update(wp,lasthor,ver,len);
			 len = 0;
		      }
		  }/*forhor*/

		  if (len) {		   /* end of this line - refresh now */
		     if (from) scr_scrolldown(wp->win_left + lasthor,
					      wp->win_top + ver,
					      len, 2,
					      1, wp->chr_attrib);
		     else      scr_update(wp,lasthor,ver,len);
		  }
	      }/*forver*/
	   }/*if*/

	   for (ver = 1; ver <= lines; ver++) {
	       win_setpos(win,1,ver);
	       win_clreol(win);
	   }
	}/*if*/

	win_setpos(win,horsav,versav);
	win_setcur(win,cursav);

	return (1);
}/*win_scrolldown()*/


int _win_exp _win_fast win_insline(win, lines)
WIN_IDX win;
byte lines;
{
	byte leftsav, topsav, rightsav, bottomsav;
	byte horsav, versav, cursav;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	win_getrange(win,&leftsav,&topsav,&rightsav,&bottomsav);
	win_getpos(win,&horsav,&versav);
	cursav = win_getcur(win);
	win_setcur(win,CUR_HIDDEN);

	win_setrange(win,leftsav,(topsav + versav) - 1,rightsav,bottomsav);
	win_scrolldown(win,lines);

	win_setrange(win,leftsav,topsav,rightsav,bottomsav);
	win_setpos(win,horsav,versav);
	win_setcur(win,cursav);

	return (1);
}/*win_insline()*/


int _win_exp _win_fast win_delline(win, lines)
WIN_IDX win;
byte lines;
{
	byte leftsav, topsav, rightsav, bottomsav;
	byte horsav, versav, cursav;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	win_getrange(win,&leftsav,&topsav,&rightsav,&bottomsav);
	win_getpos(win,&horsav,&versav);
	cursav = win_getcur(win);
	win_setcur(win,CUR_HIDDEN);

	win_setrange(win,leftsav,(topsav + versav) - 1,rightsav,bottomsav);
	win_scrollup(win,lines);

	win_setrange(win,leftsav,topsav,rightsav,bottomsav);
	win_setpos(win,horsav,versav);
	win_setcur(win,cursav);

	return (1);
}/*win_delline()*/


int _win_exp _win_fast win_inschar(win)        /* insert char at current pos */
WIN_IDX win;
{
	word ofs;
	byte width;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	ofs = (wp->cur_top + wp->cur_verpos) * wp->win_horlen + wp->cur_left + wp->cur_horpos;
	width = wp->cur_horlen - wp->cur_horpos;

	if (width > 1)
	   win_movebuf(wp->win_buf + ofs + 1, wp->win_buf + ofs, width - 1);

	*(wp->win_buf + ofs) = ((word) wp->chr_attrib << 8) | ' ';

	if (!wp->win_below) {			   /* nothing below? easier! */
	   scr_update(wp, wp->cur_left + wp->cur_horpos,
			  wp->cur_top + wp->cur_verpos,
			  width);
	}
	else if (wp->win_below < wp->win_horlen * wp->win_verlen) {
	   WIN_IDX *tpl_ptr;
	   register byte hor;
	   byte from, len;

	   tpl_ptr = win_tpl + (wp->win_top + wp->cur_top + wp->cur_verpos) * win_maxhor +
			       wp->win_left + wp->cur_left + wp->cur_horpos;

	   for (len = 0, hor = 0; hor < width; hor++) {
	       if (*(tpl_ptr + hor) == win) {		      /* is visible? */
		  if (!len) from = hor;
		  len++;
	       }
	       else if (len) {
		  scr_update(wp,wp->cur_left + wp->cur_horpos + from,
				wp->cur_top + wp->cur_verpos, len);
		  len = 0;
	       }
	   }/*forhor*/

	   if (len)			    /* end of the line - refresh now */
	      scr_update(wp,wp->cur_left + wp->cur_horpos + from,
			    wp->cur_top + wp->cur_verpos, len);
	}/*if*/

	return (1);
}/*win_inschar()*/


int _win_exp _win_fast win_delchar(win)        /* delete char at current pos */
WIN_IDX win;
{
	word ofs;
	byte width;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	ofs = (wp->cur_top + wp->cur_verpos) * wp->win_horlen + wp->cur_left + wp->cur_horpos;
	width = wp->cur_horlen - wp->cur_horpos;

	if (width > 1)
	   win_movebuf(wp->win_buf + ofs, wp->win_buf + ofs + 1, width - 1);

	*(wp->win_buf + ofs + (width - 1)) = ((word) wp->chr_attrib << 8) | ' ';

	if (!wp->win_below) {			   /* nothing below? easier! */
	   scr_update(wp, wp->cur_left + wp->cur_horpos,
			  wp->cur_top + wp->cur_verpos,
			  width);
	}
	else if (wp->win_below < wp->win_horlen * wp->win_verlen) {
	   WIN_IDX *tpl_ptr;
	   register byte hor;
	   byte from, len;

	   tpl_ptr = win_tpl + (wp->win_top + wp->cur_top + wp->cur_verpos) * win_maxhor +
			       wp->win_left + wp->cur_left + wp->cur_horpos;

	   for (len = 0, hor = 0; hor < width; hor++) {
	       if (*(tpl_ptr + hor) == win) {		      /* is visible? */
		  if (!len) from = hor;
		  len++;
	       }
	       else if (len) {
		  scr_update(wp,wp->cur_left + wp->cur_horpos + from,
				wp->cur_top + wp->cur_verpos, len);
		  len = 0;
	       }
	   }/*forhor*/

	   if (len)			    /* end of the line - refresh now */
	      scr_update(wp,wp->cur_left + wp->cur_horpos + from,
			    wp->cur_top + wp->cur_verpos, len);
	}/*if*/

	return (1);
}/*win_delchar()*/


int _win_exp _win_fast win_setpos(win, horpos, verpos)			   /* set curpos in win */
WIN_IDX win;
byte horpos, verpos;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	if (wp->con_emu & CON_UNBLANK)		/* unblank screen if allowed */
	   scr_unblank();

	if (horpos > wp->cur_horlen || verpos > wp-> cur_verlen)    /* range */
	   return (0);

	wp->cur_horpos = horpos - 1;			/* okay, set new pos */
	wp->cur_verpos = verpos - 1;

	if (win_gettop() == win)	   /* only if this is the top window */
	   scr_setpos();			    /* move cursor on screen */

	return (1);
}/*win_setpos()*/


int _win_exp _win_fast win_getpos(win, horpos, verpos)			    /* getcurpos in win */
WIN_IDX win;
byte *horpos, *verpos;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	*horpos = wp->cur_horpos + 1;		       /* okay, get position */
	*verpos = wp->cur_verpos + 1;

	return (1);
}/*win_getpos()*/


int _win_exp _win_fast win_setattrib(win, attrib)			 /* set win's attribute */
WIN_IDX win;
byte attrib;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	wp->chr_attrib = attrib;

	return (1);
}/*win_setattrib()*/


byte _win_exp _win_fast win_getattrib(win)				 /* get win's attribute */
WIN_IDX win;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	return (wp->chr_attrib);
}/*win_setattrib()*/


int _win_exp _win_fast win_addattrib(win, attrib)		    /* add new attribute to win */
WIN_IDX win;
byte attrib;
{
	byte i;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	for (i = 0; i < NUM_ATTRIBS; i++) {
	    if (attribs[i].attrib == attrib) {
	       wp->chr_attrib &= attribs[i].mask;
	       wp->chr_attrib |= attribs[i].value;
	       return (1);		/* color number found, return succes */
	    }
	}

	return (0);			       /* nothing found, return fail */
}/*win_addattrib()*/


int _win_exp cdecl win_line(win, horbegin, verbegin, horend, verend, linetype)
WIN_IDX win;
byte horbegin, verbegin;
byte horend, verend, linetype;
{				   /* draw line in win, rel to win (not cur) */
	WIN_IDX *tpl_ptr;
	byte hor, ver;
	word offset;
	byte index;
	byte c;
	word attrib;
	char *p;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	if (!horbegin || !horend || !verbegin || !verend)
	   return (0);
	if (horbegin > wp->win_horlen || horend > wp->win_horlen ||
	    verbegin > wp->win_verlen || verend > wp->win_verlen)
	   return (0);
	horbegin--; horend--; verbegin--; verend--;
	attrib = wp->chr_attrib << 8;

	if (horbegin == horend) {			    /* vertical line */
	   hor = horbegin;
	   if (verbegin > verend) {
	      ver = verend;
	      verend = verbegin;
	      verbegin = ver;
	   }
	   else
	      ver = verbegin;
	   tpl_ptr = win_tpl + (wp->win_top + verbegin) * win_maxhor +
			       wp->win_left + horbegin;
	   offset = verbegin * wp->win_horlen + horbegin;

	   while (ver <= verend) {
		 index = 0;
		 if (hor) {
		    c = wp->win_buf[offset - 1];
		    if (c >= LINE_FIRST && c <= LINE_LAST)
		       index += (((line_ascii[c - LINE_FIRST] &
				   LINE_MASK_RIGHT) >> LINE_SHIFT_RIGHT) * LINE_MUX_LEFT);
		    else if (linetype > LINE_DOUBLE && (p = strchr(line_set2,c)) != NULL)
		       index += (((line_ascii2[(byte) (p - line_set2)] &
				   LINE_MASK_RIGHT) >> LINE_SHIFT_RIGHT) * LINE_MUX_LEFT);
		 }
		 if (ver) {
		    c = wp->win_buf[offset - wp->win_horlen];
		    if (c >= LINE_FIRST && c <= LINE_LAST)
		       index += (((line_ascii[c - LINE_FIRST] &
				   LINE_MASK_BOTTOM) >> LINE_SHIFT_BOTTOM) * LINE_MUX_TOP);
		    else if (linetype > LINE_DOUBLE && (p = strchr(line_set2,c)) != NULL)
		       index += (((line_ascii2[(byte) (p - line_set2)] &
				   LINE_MASK_BOTTOM) >> LINE_SHIFT_BOTTOM) * LINE_MUX_TOP);
		 }
		 if (hor < (wp->win_horlen - 1)) {
		    c = wp->win_buf[offset + 1];
		    if (c >= LINE_FIRST && c <= LINE_LAST)
		       index += (((line_ascii[c - LINE_FIRST] &
				   LINE_MASK_LEFT) >> LINE_SHIFT_LEFT) * LINE_MUX_RIGHT);
		    else if (linetype > LINE_DOUBLE && (p = strchr(line_set2,c)) != NULL)
		       index += (((line_ascii2[(byte) (p - line_set2)] &
				   LINE_MASK_LEFT) >> LINE_SHIFT_LEFT) * LINE_MUX_RIGHT);
		 }
		 if (ver < verend)
		    index += ((linetype & ~LINE_ASCII) * LINE_MUX_BOTTOM);
		 else if (ver < (wp->win_verlen - 1)) {
		    c = wp->win_buf[offset + wp->win_horlen];
		    if (c >= LINE_FIRST && c <= LINE_LAST)
		       index += (((line_ascii[c - LINE_FIRST] &
				   LINE_MASK_TOP) >> LINE_SHIFT_TOP) * LINE_MUX_BOTTOM);
		    else if (linetype > LINE_DOUBLE && (p = strchr(line_set2,c)) != NULL)
		       index += (((line_ascii2[(byte) (p - line_set2)] &
				   LINE_MASK_TOP) >> LINE_SHIFT_TOP) * LINE_MUX_BOTTOM);
		 }
		 wp->win_buf[offset] = attrib | ((linetype & LINE_ASCII) ?
						 line_table2[index] : line_table[index]);
		 if (*tpl_ptr == win)
		    scr_update(wp,horbegin,ver,1);
		 tpl_ptr += win_maxhor;
		 offset += wp->win_horlen;
		 ver++;
	   }
	}

	else if (verbegin == verend) {			  /* horizontal line */
	   ver = verbegin;
	   if (horbegin > horend) {
	      hor = horend;
	      horend = horbegin;
	      horbegin = hor;
	   }
	   else
	      hor = horbegin;
	   tpl_ptr = win_tpl + (wp->win_top + verbegin) * win_maxhor +
			       wp->win_left + horbegin;
	   offset = verbegin * wp->win_horlen + horbegin;

	   while (hor <= horend) {
		 index = 0;
		 if (hor) {
		    c = wp->win_buf[offset - 1];
		    if (c >= LINE_FIRST && c <= LINE_LAST)
		       index += (((line_ascii[c - LINE_FIRST] &
				   LINE_MASK_RIGHT) >> LINE_SHIFT_RIGHT) * LINE_MUX_LEFT);
		    else if (linetype > LINE_DOUBLE && (p = strchr(line_set2,c)) != NULL)
		       index += (((line_ascii2[(byte) (p - line_set2)] &
				   LINE_MASK_RIGHT) >> LINE_SHIFT_RIGHT) * LINE_MUX_LEFT);
		 }
		 if (ver) {
		    c = wp->win_buf[offset - wp->win_horlen];
		    if (c >= LINE_FIRST && c <= LINE_LAST)
		       index += (((line_ascii[c - LINE_FIRST] &
				   LINE_MASK_BOTTOM) >> LINE_SHIFT_BOTTOM) * LINE_MUX_TOP);
		    else if (linetype > LINE_DOUBLE && (p = strchr(line_set2,c)) != NULL)
		       index += (((line_ascii2[(byte) (p - line_set2)] &
				   LINE_MASK_BOTTOM) >> LINE_SHIFT_BOTTOM) * LINE_MUX_TOP);
		 }
		 if (hor < horend)
		    index += ((linetype & ~LINE_ASCII) * LINE_MUX_RIGHT);
		 else if (hor < (wp->win_horlen - 1)) {
		    c = wp->win_buf[offset + 1];
		    if (c >= LINE_FIRST && c <= LINE_LAST)
		       index += (((line_ascii[c - LINE_FIRST] &
				   LINE_MASK_LEFT) >> LINE_SHIFT_LEFT) * LINE_MUX_RIGHT);
		    else if (linetype > LINE_DOUBLE && (p = strchr(line_set2,c)) != NULL)
		       index += (((line_ascii2[(byte) (p - line_set2)] &
				   LINE_MASK_LEFT) >> LINE_SHIFT_LEFT) * LINE_MUX_RIGHT);
		 }
		 if (ver < (wp->win_verlen - 1)) {
		    c = wp->win_buf[offset + wp->win_horlen];
		    if (c >= LINE_FIRST && c <= LINE_LAST)
		       index += (((line_ascii[c - LINE_FIRST] &
				   LINE_MASK_TOP) >> LINE_SHIFT_TOP) * LINE_MUX_BOTTOM);
		    else if (linetype > LINE_DOUBLE && (p = strchr(line_set2,c)) != NULL)
		       index += (((line_ascii2[(byte) (p - line_set2)] &
				   LINE_MASK_TOP) >> LINE_SHIFT_TOP) * LINE_MUX_BOTTOM);
		 }
		 wp->win_buf[offset] = attrib | ((linetype & LINE_ASCII) ?
						 line_table2[index] : line_table[index]);
		 offset++;
		 hor++;
	   }

	   if (wp->win_status != WIN_HIDDEN) {
	      if (!wp->win_below)
		 scr_update(wp,horbegin,verbegin,(horend - horbegin) + 1);
	      else {
		 /*register*/ byte lasthor, len;

		 for (len = lasthor = 0, hor = horbegin;
		      hor <= horend;
		      tpl_ptr++, hor++) {
		     if (*tpl_ptr == win) {		  /* is pos visible? */
			if (!len)		       /* add to refresh buf */
			   lasthor = hor;
			len++;
		     }

		     else if (len) {
			scr_update(wp,lasthor,verbegin,len);
			len = 0;
		     }
		 }/*forhor*/

		 if (len)			/* end of line - refresh now */
		    scr_update(wp,lasthor,verbegin,len);
	      }
	   }
	}
	else
	   return (0);				  /* no diagonals please ;-) */

	if (wp->con_emu & CON_UNBLANK)
	   scr_unblank();

	return (1);
}/*win_line()*/


int _win_exp _win_fast win_fill(win, fillc)    /* fill window with specified char in cur attrib */
WIN_IDX win;
char fillc;
{
	register byte hor, ver;
	word c;

	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	c = ((word) wp->chr_attrib << 8) | (byte) fillc;     /* set fillchar */

	if (wp->cur_horlen == wp->win_horlen)	/* clr entire window width ? */
	   win_setbuf(wp->win_buf + wp->cur_top * wp->win_horlen,
		      c, wp->cur_verlen * wp->win_horlen);
	else					   /* nope, clr line by line */
	   for (ver = wp->cur_top; ver <= wp->cur_bottom; ver++)
	       win_setbuf(wp->win_buf + ver * wp->win_horlen + wp->cur_left,
			  c, wp->cur_horlen);

	if (wp->win_status == WIN_HIDDEN)		/* that's easy stuff */
	   return (1);

	if (!wp->win_below) {				 /* topwin is easier */
	   if (wp->cur_horlen == win_maxhor)	 /* update entire screen width? */
	      scr_update(wp,0,wp->cur_top,wp->cur_verlen * win_maxhor);
	   else {				      /* update line by line */
	      for (ver = wp->cur_top; ver <= wp->cur_bottom; ver++)
		  scr_update(wp,wp->cur_left,ver,wp->cur_horlen);
	   }
	}

	else {					       /* not the top window */
	   WIN_IDX *tpl_ptr;
	   byte lasthor, len;

	   tpl_ptr = win_tpl +
		     (wp->win_top + wp->cur_top) * win_maxhor +
		     wp->win_left;
	   lasthor = 0;

	   for (ver = wp->cur_top;
		ver <= wp->cur_bottom;
		tpl_ptr += win_maxhor, ver++) {

	       for (len = 0, hor = wp->cur_left;	  /* scan every line */
		    hor <= wp->cur_right;
		    hor++) {
		   if (*(tpl_ptr + hor) == win) {	  /* is pos visible? */
		      if (!len) 		       /* add to refresh buf */
			 lasthor = hor;
		      len++;
		   }

		   else if (len) {
		      scr_update(wp,lasthor,ver,len);
		      len = 0;
		   }
	       }/*forhor*/

	       if (len) 		   /* end of this line - refresh now */
		  scr_update(wp,lasthor,ver,len);
	   }/*forver*/
	}

	if (wp->con_emu & CON_UNBLANK)
	   scr_unblank();

	return (1);
}/*win_fill()*/


int _win_exp _win_fast win_putc(win, c)   /* print char in window at cur pos */
WIN_IDX win;
char c;
{
	static char *s = " ";

	*s = c;
	return (win_puts(win,s));
}/*win_putc()*/


int _win_exp _win_fast win_puts(win, s) 		 /* print string in window from cur pos */
WIN_IDX win;
char *s;
{
	if (windows == NULL)
	   return (0);
	if (win >= num_windows)
	   return (0);

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	con_puts(s);

	return (1);
}/*win_puts()*/


int _win_exp _win_fast win_xyputs(win, horpos, verpos, s)
WIN_IDX win;
byte horpos, verpos;
char *s;
{			    /* print string in window from specified cur pos */
	if (!win_setpos(win,horpos,verpos))
	   return (0);

	con_puts(s);

	return (1);
}/*win_puts()*/


/* end of window.c */
