/* AGLMOUSE - functions for Microsoft and compatible Mouse drivers
 * Copyright (C) 1989-2001 Arjen G. Lentz
 *
 * This file is part of AGLMOUSE.
 * AGLMOUSE is free software; you can redistribute it and/or
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


/* mouse.h - Definitions for Microsoft and compatible Mouse-drivers */
#if	!defined(__MOUSE_DEF_)
#define __MOUSE_DEF_


#define LEFTBUTTON   0x01
#define RIGHTBUTTON  0x02
#define MIDDLEBUTTON 0x04

#define MOUSEMOVE     0x01
#define LEFTPRESS     0x02
#define LEFTRELEASE   0x04
#define RIGHTPRESS    0x08
#define RIGHTRELEASE  0x10
#define MIDDLEPRESS   0x20
#define MIDDLERELEASE 0x40


typedef struct {
	unsigned int ax, bx, cx, dx, es;
} MOUSE_REGS;

typedef struct {
	int horhotspot;
	int verhotspot;
	unsigned int scrmask[16];
	unsigned int curmask[16];
} MOUSE_CURBITMAP;

#ifdef __cplusplus
extern "C" {
#endif
typedef void far pascal (*MOUSE_EVENTHANDLER) (void);
#ifdef	__cplusplus
}
#endif

typedef char *MOUSE_STATEBUF;

#if !defined(__MOUSE_MAIN_)
extern MOUSE_REGS pascal mouse_regs;
extern MOUSE_CURBITMAP pascal mouse_standardpointer,
		       pascal mouse_uparrow,
		       pascal mouse_downarrow,
		       pascal mouse_leftarrow,
		       pascal mouse_rightarrow,
		       pascal mouse_checkmark,
		       pascal mouse_pointinghand;
#endif


#ifdef __cplusplus
extern "C" {
#endif

void _win_fast mouse_int	    (int mousefunc);
int  _win_fast mouse_init	    (void);

int  _win_fast mouse_buttons	    (void);
int  _win_fast mouse_press	    (int button, int *pressed, int *horpos, int *verpos);
int  _win_fast mouse_release	    (int button, int *released, int *horpos, int *verpos);

void _win_fast mouse_cursor	    (int flag);
void _win_fast mouse_softtextcursor (int scrmask, int curmask);
void _win_fast mouse_hardtextcursor (int startscanline, int endscanline);
void _win_fast mouse_graphcursor    (MOUSE_CURBITMAP *curbitmap);

void _win_fast mouse_sensitivity    (int hormickeys, int vermickeys, int threshold);
void _win_fast mouse_window	    (int left, int top, int right, int bottom);
void _win_fast mouse_disable	    (int left, int top, int right, int bottom);
void _win_fast mouse_gotoxy	    (int horpos, int verpos);
int  _win_fast mouse_wherexy	    (int *horpos, int *verpos);
void _win_fast mouse_motion	    (int *hormickeys, int *vermickeys);

void _win_fast mouse_installhandler (int callmask, MOUSE_EVENTHANDLER evtfuncptr);
int  _win_fast mouse_swaphandlers   (int callmask, MOUSE_EVENTHANDLER evtfuncptr, MOUSE_EVENTHANDLER *oldfuncptr);

int  _win_fast mouse_statesize	    (void);
void _win_fast mouse_savestate	    (MOUSE_STATEBUF statebufptr);
void _win_fast mouse_restorestate   (MOUSE_STATEBUF statebufptr);

int  _win_fast mouse_getcrtpage     (void);
void _win_fast mouse_setcrtpage     (int crtpage);

void _win_fast mouse_lightpen	    (int flag);

#ifdef	__cplusplus
}
#endif

#endif


/* end of mouse.h */
