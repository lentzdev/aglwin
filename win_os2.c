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


/* win_os2.c	- OS/2 dependent functions (AGL 1993) */
#define INCL_BASE
#if defined(__BORLANDC__)
#define INCL_NOPMAPI
#else
#define INCL_NOPM
#endif
#include <os2.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include "window.h"


#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#define MODE_BW40	   0		/* Black & white, 40 columns	     */
#define MODE_C40	   1		/* Color, 40 columns		     */
#define MODE_BW80	   2		/* Black & white, 80 columns	     */
#define MODE_C80	   3		/* Color, 80 columns		     */
#define MODE_MONO	   7		/* Monochrome, 80 columns	     */

#define SCR_BLANKED	0x0020		/* Screen is currently blanked	     */
#define SCR_STATESAVED	0x0040		/* State saved - system disabled     */

enum { VDU_UNKNOWN, VDU_MONO, VDU_CGA, VDU_EGA, VDU_VGACOLOUR, VDU_VGAMONO
};

enum { MT_NONE, MT_ATBIOS, MT_DDOS, MT_DESQVIEW, MT_TOPVIEW, MT_MULTILINK,
       MT_PCMOS, MT_WIN386, MT_WIN3, MT_WIN3E, MT_OS2
};


static SCR_PTR	work_buf;		/* pointer to work/blank buffer      */
static word	scr_flags;		/* display types and writing methods */
static word	cur_normalattrib,	/* cursor shapes normal and insert   */
		cur_insertattrib;
#if 0
static volatile word ctrl_brk;


void break_handler (int sig)
{
	sig = sig;

	signal(SIGINT,SIG_IGN);
	ctrl_brk++;
	signal(SIGINT,break_handler);
}/*break_handler()*/
#endif


int _win_exp _win_fast scr_init(void)		       /* init screen system */
{
	VIOMODEINFO vio_mode;
	USHORT horpos, verpos;
	USHORT scrsize;

#if 0
	ctrl_brk = 0;
	signal(SIGINT,break_handler);
#endif
	signal(SIGINT,SIG_IGN);

	scr_flags = 0;				   /* reset all screen flags */

	vio_mode.cb = sizeof (VIOMODEINFO);
	VioGetMode(&vio_mode,0);

	if ((vio_mode.fbType & 0x02) ||
	    (vio_mode.col < 80 || vio_mode.row < 25)) {
	   vio_mode.fbType &= ~0x02;		       /* switch to textmode */
	   vio_mode.col = 80;
	   vio_mode.row = 25;
	   VioSetMode(&vio_mode,0);
	}
	win_maxhor = vio_mode.col;
	win_maxver = vio_mode.row;

	cur_normalattrib = 0x0e0f;
	cur_insertattrib = 0x0a0f;

	VioGetCurPos(&verpos,&horpos,0);	  /* get current cursor pos  */
	windows->cur_horpos = (byte) horpos;	  /* set window 0 cursor pos */
	windows->cur_verpos = (byte) verpos;

	/* now read the current screen contents into window 0 */
	windows->win_buf = (SCR_PTR) malloc(sizeof (SCR_CELL) * win_maxhor * win_maxver);
	if (windows->win_buf == NIL)
	   return (0);
	win_setbuf(windows->win_buf,0x0720,win_maxhor * win_maxver);
	scrsize = (USHORT) (win_maxhor * win_maxver * sizeof (SCR_CELL));
	VioReadCellStr((PCH) windows->win_buf,
		       (PUSHORT) &scrsize, (USHORT) 0, (USHORT) 0, (HVIO) 0);

#if 0
	/* trying to get rid of the garbage screen problem... */
	VioSetMode(&vio_mode,0);
	VioScrollDn(0,0,-1,-1,-1," \7",0);
	VioWrtCellStr((PCH) windows->win_buf,
		      (USHORT) scrsize, (USHORT) 0, (USHORT) 0, (HVIO) 0);
#endif

	work_buf = (SCR_PTR) malloc(sizeof (SCR_CELL) * win_maxhor * win_maxver);
	if (work_buf == NIL)
	   return (0);
	win_movebuf(work_buf,windows->win_buf,win_maxhor * win_maxver);

	return (1);
}/*scr_init()*/


void _win_exp _win_fast scr_deinit(void)		      /* deinit screen system */
{
	if (scr_flags & SCR_STATESAVED)
	   (void) win_restorestate();

	free(work_buf);
}/*scr_deinit()*/


void _win_exp _win_fast scr_savestate(void)  /* save entire screen/mouse state and reset */
{
	VIOCURSORINFO vio_curinfo;

	if (windows == NULL || (scr_flags & SCR_STATESAVED))
	   return;

	scr_unblank();

	win_keypurge();

	VioWrtCellStr((PCH) windows->win_buf,
		      (USHORT) (win_maxhor * win_maxver * sizeof (SCR_CELL)),
		      (USHORT) 0, (USHORT) 0, (HVIO) 0);

	if (windows->cur_type == CUR_INSERT) {
	   vio_curinfo.yStart = -70;
	   vio_curinfo.cEnd   = -100;
	   vio_curinfo.attr   = 0;
	}
	else if (windows->cur_type == CUR_NONE) {
	   vio_curinfo.yStart = vio_curinfo.cEnd = 1;
	   vio_curinfo.attr   = -1;
	}
	else {
	   vio_curinfo.yStart = -90;
	   vio_curinfo.cEnd   = -100;
	   vio_curinfo.attr   = 0;
	}

	vio_curinfo.cx = 1;
	VioSetCurType((void *) &vio_curinfo,0);

	VioSetCurPos(windows->win_top + windows->cur_top + windows->cur_verpos,
		     windows->win_left + windows->cur_left + windows->cur_horpos,
		     0);

	scr_flags |= SCR_STATESAVED;
}/*scr_savestate()*/


void _win_exp _win_fast scr_restorestate(void)	/* restore previously saved screen state */
{
	if (windows == NULL || !(scr_flags & SCR_STATESAVED))
	   return;

	scr_flags &= ~SCR_STATESAVED;

	VioWrtCellStr((PCH) work_buf,
		      (USHORT) (win_maxhor * win_maxver * sizeof (SCR_CELL)),
		      (USHORT) 0, (USHORT) 0, (HVIO) 0);

	scr_setcur();
	scr_setpos();

	win_keypurge(); 		       /* reset keyboard ring buffer */
}/*scr_restorestate()*/


void _win_exp _win_fast scr_blank(void)        /* blank system screen and hide cursor */
{
	SCR_CELL blankcell = 0;
	VIOCURSORINFO vio_curinfo;

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	vio_curinfo.yStart = vio_curinfo.cEnd = vio_curinfo.cx = 1;
	vio_curinfo.attr   = -1;
	VioSetCurType((void *) &vio_curinfo,0);

	VioWrtNCell((void *) &blankcell,win_maxhor * win_maxver,0,0,0);

	scr_flags |= SCR_BLANKED;
}/*scr_blank()*/


void _win_exp _win_fast scr_unblank(void)	    /* unblank system screen and restore cursor */
{
	if (!(scr_flags & SCR_BLANKED))
	   return;

	VioWrtCellStr((PCH) work_buf,
		      (USHORT) (win_maxhor * win_maxver * sizeof (SCR_CELL)),
		      (USHORT) 0, (USHORT) 0, (HVIO) 0);

	scr_flags &= ~SCR_BLANKED;
	scr_setcur();
}/*scr_unblank()*/


void _win_exp _win_fast scr_bell(void)	       /* make bell (ASCII 07) sound one way or another */
{
	if (scr_flags & SCR_STATESAVED)
	   return;

	VioWrtTTY("\007",1,0);
}/*scr_bell()*/


void _win_exp _win_fast scr_update(WIN_REC *wp, byte hor, byte ver, int len)
{
	win_movebuf(work_buf + (wp->win_top + ver) * win_maxhor +
			       wp->win_left + hor,
		    wp->win_buf + ver * wp->win_horlen + hor,len);

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	VioWrtCellStr((PCH) (wp->win_buf + ver * wp->win_horlen + hor),
		      (USHORT) (len * sizeof (SCR_CELL)),
		      (USHORT) (wp->win_top + ver),
		      (USHORT) (wp->win_left + hor), (HVIO) 0);
}/*scr_update()*/


void _win_exp _win_fast scr_scrollup (byte left, byte top, byte width, byte depth,
			    byte lines, byte fillattrib)
{
	SCR_CELL c = ((word) fillattrib << 8) | (byte) ' ';
	byte ver;

	if (width == win_maxhor) {
	   win_movebuf(work_buf + top * win_maxhor,
		       work_buf + (top + lines) * win_maxhor,
		       width * (depth - lines));
	   win_setbuf(work_buf + ((top + depth) - lines) * win_maxhor,c,width * lines);
	}
	else {
	   for (ver = top; ver <= (top + (depth - 1)) - lines; ver++)
	       win_movebuf(work_buf + ver * win_maxhor + left,
			   work_buf + (ver + lines) * win_maxhor + left,
			   width);
	   for (ver = (top + depth) - lines; ver < top + depth; ver++)
	       win_setbuf(work_buf + ver * win_maxhor + left,c,width);
	}

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	VioScrollUp(top,left,(top + depth) - 1,(left + width) - 1,
		    (USHORT) lines,(PBYTE) &c,0);
}/*scr_scrollup()*/


void _win_exp _win_fast scr_scrolldown (byte left, byte top, byte width, byte depth,
			      byte lines, byte fillattrib)
{
	SCR_CELL c = ((word) fillattrib << 8) | (byte) ' ';
	byte ver;

	if (width == win_maxhor) {
	   win_movebuf(work_buf + (top + lines) * win_maxhor,
		       work_buf + top * win_maxhor,
		       width * (depth - lines));
	   win_setbuf(work_buf + top * win_maxhor,c,width * lines);
	}
	else {
	   for (ver = top + (depth - 1); ver >= top + lines; ver--)
	       win_movebuf(work_buf + ver * win_maxhor + left,
			   work_buf + (ver - lines) * win_maxhor + left,
			   width);
	   for (ver = top + lines - 1; ver >= top; ver--)
	       win_setbuf(work_buf + ver * win_maxhor + left,c,width);
	}

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	VioScrollDn(top,left,(top + depth) - 1,(left + width) - 1,
		    (USHORT) lines,(PBYTE) &c,0);
}/*scr_scrolldown()*/


void _win_exp _win_fast scr_setcur(void)      /* set cur shape/size to cur_status of top window */
{
	byte curtype;
	VIOCURSORINFO vio_curinfo;

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	curtype = win_getwp(win_gettop())->cur_type;

	if (curtype == CUR_NORMAL) {
	   vio_curinfo.yStart = -90;
	   vio_curinfo.cEnd   = -100;
	   vio_curinfo.attr   = 0;
	}
	else if (curtype == CUR_INSERT) {
	   vio_curinfo.yStart = -70;
	   vio_curinfo.cEnd   = -100;
	   vio_curinfo.attr   = 0;
	}
	else if (curtype == CUR_NONE) {
	   vio_curinfo.yStart = vio_curinfo.cEnd = 1;
	   vio_curinfo.attr   = -1;
	}
	else
	   return;

	vio_curinfo.cx = 1;
	VioSetCurType(&vio_curinfo,0);
}/*scr_setcur()*/


void _win_exp _win_fast scr_setpos(void)
{
	WIN_REC *wp;

	if (scr_flags & SCR_STATESAVED)
	   return;

	wp = win_getwp(win_gettop());
	VioSetCurPos(wp->win_top + wp->cur_top + wp->cur_verpos,
		     wp->win_left + wp->cur_left + wp->cur_horpos,
		     0);
}/*scr_setpos()*/


void _win_exp _win_fast win_setbuf(SCR_PTR dst, SCR_CELL c, int len)
{
	if (!len) return;

	while (len)
	      dst[--len] = c;

#if 0
	asm	mov	ecx, [len]
	asm	jecxz	win_setbuf_exit
	asm	mov	edi, [dst]
	asm	mov	ax, [c]
	asm	cld
	asm	rep	stosw
win_setbuf_exit:
	return;

	memset(dst,c,len * sizeof (SCR_CELL));

	asm	mov	cx, len
	asm	jcxz	win_setbuf_exit
	asm	les	di, dst
	asm	mov	ax, c
	asm	cld
	asm	rep	stosw
win_setbuf_exit:
	return;
#endif
}/*win_setbuf()*/


void _win_exp _win_fast win_movebuf(SCR_PTR dst, SCR_PTR src, int len)
{
	memmove(dst,src,len * sizeof (SCR_CELL));

#if 0
	asm	mov	cx, len 	/* Length value into CX */
	asm	jcxz	win_movebuf_exit

	asm	push	ds
	asm	les	di, dst 	/* Get pointers to data area */
	asm	lds	si, src

	asm	cld			/* Setup move direction */
	asm	cmp	si, di		/* Check for move direction */
	asm	jae	win_movebuf_start/*Moving down? then forward move ok */
	asm	mov	ax, cx		/* Nope, then start at other end */
	asm	dec	ax
	asm	shl	ax, 1
	asm	add	si, ax
	asm	add	di, ax
	asm	std

win_movebuf_start:
	asm	rep	movsw
	asm	pop	ds
win_movebuf_exit:
	return;
#endif
}/*win_movebuf()*/


void _win_exp cdecl win_timeslice(void)
{
	DosSleep(1);
}/*win_timeslice()*/


word _win_exp _win_fast win_getvdutype (void)
{
	VIOCONFIGINFO vio_config;

	vio_config.cb = sizeof (VIOCONFIGINFO);
	VioGetConfig(VIO_CONFIG_CURRENT,&vio_config,0);
	switch (vio_config.adapter) {
	       case DISPLAY_MONOCHROME: return (VDU_MONO);
	       case DISPLAY_CGA:	return (VDU_CGA);
	       case DISPLAY_EGA:	return (VDU_EGA);
	       case DISPLAY_VGA:
	       case DISPLAY_8514A:	switch (vio_config.display) {
					       case MONITOR_MONOCHROME:
						    return (VDU_VGAMONO);
					       default:
						    return (VDU_VGACOLOUR);
					}
	       default: 		/*return (VDU_UNKNOWN);*/
					return (VDU_VGACOLOUR);
	}
}/*win_getvdutype()*/


word _win_exp _win_fast win_getmttype (void)
{
	return (MT_OS2);
}/*win_getmttype()*/


void _win_exp _win_fast win_setvdudirect (int direct)
{
	direct = direct;  /* just to remove warning */
}/*win_setvdudirect()*/


void _win_exp _win_fast key_fillbuf(void)
{
	word scancode;

	if (scr_flags & SCR_STATESAVED)
	   return;

#if 0
	while (!win_keyfull() && ctrl_brk && win_keyputc(Ctrl_C))
	      ctrl_brk--;
#endif

	while (!win_keyfull() && kbhit()) {
	      scancode = getch();
	      if (!win_keyputc(scancode && scancode != 0xE0 ?
			       scancode : 0x100 | getch()))
		    break;
	}
}/*key_fillbuf()*/


int _win_exp _win_fast mcur_enable (void)
{
	return (0);
}/*mcur_enable()*/


void _win_exp _win_fast mcur_disable (void)
{
	return;
}/*mcur_disable()*/


void _win_exp _win_fast mcur_show (void)
{
	return;
}/*mcur_show()*/


void _win_exp _win_fast mcur_hide (void)
{
	return;
}/*mcur_hide()*/


void _win_exp _win_fast mcur_reset (void)
{
	return;
}/*mcur_reset()*/

void _win_exp _win_fast mcur_setrange (byte left, byte top, byte right, byte bottom)
{
	left = left; top = top; right = right; bottom = bottom;

	return;
}/*mcur_setrange()*/


void _win_exp _win_fast mcur_getrange (byte *left, byte *top, byte *right, byte *bottom)
{
	left = left; top = top; right = right; bottom = bottom;

	return;
}/*mcur_getrange()*/


void _win_exp _win_fast mcur_setpos (byte horpos, byte verpos)
{
	horpos = horpos; verpos = verpos;

	return;
}/*mcur_setpos()*/


void _win_exp _win_fast mcur_getpos (byte *horpos, byte *verpos)
{
	horpos = horpos; verpos = verpos;

	return;
}/*mcur_getpos()*/


word _win_exp _win_fast mcur_status (void)
{
	return (0);
}/*mcur_status()*/


word _win_exp _win_fast mcur_buttons (void)
{
	return (0);
}/*mcur_buttons()*/


WIN_IDX _win_exp _win_fast mcur_getwin (void)
{
	return (0);
}/*mcur_getwin()*/


void _win_exp _win_fast mcur_getwinpos (byte *horpos, byte *verpos)
{
	horpos = horpos; verpos = verpos;

	return;
}/*mcur_getwinpos()*/


int _win_exp _win_fast mcur_getcurpos (byte *horpos, byte *verpos)
{
	horpos = horpos; verpos = verpos;

	return (0);
}/*mcur_getcurpos()*/


/* end of win_os2.c */
