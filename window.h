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


/* window.h - general global definitions	*/
#ifndef __WINDOW_DEF_
#define __WINDOW_DEF_
#include <stddef.h>
#ifdef __PROTO__
#define PROTO(s) s
#include <stdarg.h>
#else
#define PROTO(s) ()
#endif
#include "2types.h"

#if defined(__OS2__) || defined(__NT__)
#pragma pack(1)
#endif

#if !defined(__MSDOS__)
#ifdef near
#undef near
#endif
#define near
#if !defined(__OS2__)
#   if !defined(__NT__)
#      define pascal
#   endif
#define cdecl
#endif
#endif

#include "win_keys.h"
/* sub-include win_menu at bottom of if: needs declarations from this file */

#if defined(__MSDOS__)
#	if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
#	   define USEFAR
#	endif
#endif
#if defined(__OS2__) || defined(__NT__)
#	define _win_exp  _export
#else
#	define _win_exp
#endif
#if defined(__MSDOS__) || defined(__OS2__)
#define _win_fast pascal
#else
#define _win_fast
#endif
#ifdef	__TOS__
#	define win_keyputc    wk_putc
#	define win_keyputs    wk_puts
#	define win_keypush    wk_push
#	define win_scrollup   win_scup
#	define win_scrolldown win_scdo
#	define win_getcur     win_gcur
#	define win_setcur     win_scur
#	define win_clrbos     win_cbos
#	define win_clreos     win_ceos
#	define win_clrbol     win_cbol
#	define win_clreol     win_ceol
#endif
#include "far.h"

#ifndef NULL
#	ifdef __MSDOS__
#	      if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
#		 define NULL ((void *) 0)
#	      else
#		 define NULL ((void *) 0L)
#	      endif
#	else
#	      define NULL ((char *) 0L)
#	endif
#endif

#ifndef inl_toupper
#define inl_toupper(c) (islower(c) ? _toupper(c) : (c))
#endif

#ifndef inl_tolower
#define inl_tolower(c) (isupper(c) ? _tolower(c) : (c))
#endif

typedef word	      SCR_CELL; 	/* typedef screencell (for sizeof)   */
typedef SCR_CELL FAR *SCR_PTR;		/* typedef pointer to screen buffer  */
typedef byte	      WIN_IDX;

typedef struct {
	byte	win_status;   /* --------- window status (see flags below)   */
	byte	win_left,		/* left offset of window in screen   */
		win_top,		/* top	  ...			     */
		win_right,		/* right  ...			     */
		win_bottom;		/* bottom ...			     */
	byte	win_horlen,		/* mul* factor (ver * width + hor)   */
		win_verlen;		/* vertical number of lines in win   */
	word	win_below;		/* no. chars NOT VISIBLE on screen   */
	SCR_PTR win_buf;		/* pointer to window buffer	     */
	byte	cur_type;     /* --------- cursor type (none/norm/insert)    */
	byte	cur_left,		/* left offset of cursor in window   */
		cur_top,		/* top	  ...			     */
		cur_right,		/* right  ...			     */
		cur_bottom;		/* bottom ...			     */
	byte	cur_horlen,		/* hor number of cursor lines in win */
		cur_verlen;		/* ver number of cursor lines in win */
	byte	cur_horpos,		/* hor pos of cursor in cur window   */
		cur_verpos;		/* ver pos of cursor in cur window   */
	byte	con_emu;      /* --------- terminal emulation (ANSI, etc)    */
	byte	con_seqlen;		/* length of buffered emu sequence   */
	byte   *con_seqbuf;		/* char buffer for emu sequences     */
	byte	con_horsav,		/* saved hor pos (ANSI save pos seq) */
		con_versav;		/* saved ver pos (ANSI save pos seq) */
	byte	chr_attrsav;		/* saved attrib  (ANSI save cur seq) */
	byte	chr_attrib;   /* --------- current attrib for printing chars */
	byte	key_emu;      /* --------- keyboard emulation (ANSI, etc)    */
} WIN_REC;

typedef struct {
	word	status;       /* --------- mouse status (see flags below)    */
	byte	scr_left,		/* left offset of mouse range in win */
		scr_top,		/* top	  ...			     */
		scr_right,		/* right  ...			     */
		scr_bottom;		/* bottom ...			     */
	long	doubletimer;		/* UNIX timestamp of left left click */
	byte	scr_horpos,		/* hor pos on screen		     */
		scr_verpos;		/* ver pos on screen		     */
	WIN_IDX win;			/* window at last left button click  */
	byte	win_horpos,		/* hor pos within above window	     */
		win_verpos;		/* ver pos within above window	     */
	byte	cur_horpos,		/* hor pos within win's cursor range */
		cur_verpos;		/* ver pos within win's cursor range */
} WIN_MOUSE;


#define MAX_SEQ     255 		/* maximum length of emu sequences   */

#define WIN_FREE	   0		/* window is closed		     */
#define WIN_ACTIVE	   1		/* window is active and visible      */
#define WIN_HIDDEN	   2		/* window is active but hidden	     */

#define CUR_NONE	   0		/* cursor not visible in this window */
#define CUR_HIDDEN	   0		/* ditto, hide cursor		     */
#define CUR_NORMAL	   1		/* normal underscore cursor	     */
#define CUR_INSERT	   2		/* insert cursor (thick underscore)  */

#define CON_RAW 	   0		/* no char xlat except CR13 and LF10 */
#define CON_COOKED	   1		/* normal, no special char/seq xlat  */
#define CON_ANSI	   2		/* emulate IBM-ANSI/VT-100 sequences */
#define CON_VT52	   3		/* emulate VT-52 sequences	     */
#define CON_AVATAR	   4		/* emulate Avatar/IBM-ANSI/VT-100    */
#define CON_EMUMASK	0x0f		/* bits used for emulation,mask wrap */
#define CON_WRAP	0x10		/* wrap ch past end of line to next  */
#define CON_SCROLL	0x20		/* scroll if linefeed on bottom line */
#define CON_INSERT	0x40		/* insert, chs on same line to right */
#define CON_UNBLANK	0x80		/* unblank scr if print in this win  */

#define KEY_NONE	   0		/* basically none is the same as raw */
#define KEY_RAW 	   0		/* all raw, no keycode translation   */
#define KEY_ANSI	   1		/* ANSI (VT-100) keyboard emulation  */
#define KEY_VT52	   2		/* VT-52 keyboard emulation	     */

#define CHR_INVISIBLE	0x00
#ifdef __TOS__
#define CHR_NORMAL	0x03
#define CHR_INVERSE	0x83
#define CHR_BRIGHT	0x83
#define CHR_BOLD	0x83
#define CHR_UNDERLINE	0x02
#define CHR_BLINK	0x02
#define CHR_F_BLACK	0x00
#define CHR_F_BLUE	0x03
#define CHR_F_GREEN	0x03
#define CHR_F_CYAN	0x03
#define CHR_F_RED	0x03
#define CHR_F_MAGENTA	0x03
#define CHR_F_BROWN	0x03
#define CHR_F_WHITE	0x03
#define CHR_F_GREY	0x03
#define CHR_F_YELLOW	0x03
#define CHR_B_BLACK	0x00
#define CHR_B_BLUE	0x03
#define CHR_B_GREEN	0x03
#define CHR_B_CYAN	0x03
#define CHR_B_RED	0x03
#define CHR_B_MAGENTA	0x03
#define CHR_B_BROWN	0x03
#define CHR_B_WHITE	0x03

#define ATR_NORMAL	   0
#define ATR_BOLD	   1
#define ATR_UNDERLINE	   2
#define ATR_BLINK	   0
#define ATR_REVERSE	   0x80
#define ATR_INVISIBLE	   0
#define ATR_F_BLACK	  00
#define ATR_F_BLUE	  31
#define ATR_F_GREEN	  32
#define ATR_F_CYAN	  33
#define ATR_F_RED	  34
#define ATR_F_MAGENTA	  35
#define ATR_F_YELLOW	  36
#define ATR_F_WHITE	  37
#define ATR_B_BLACK	  40
#define ATR_B_BLUE	  41
#define ATR_B_GREEN	  42
#define ATR_B_CYAN	  43
#define ATR_B_RED	  44
#define ATR_B_MAGENTA	  45
#define ATR_B_YELLOW	  46
#define ATR_B_WHITE	  47
#else
#define CHR_NORMAL	0x07
#define CHR_INVERSE	0x70
#define CHR_BRIGHT	0x08
#define CHR_BOLD	0x08
#define CHR_UNDERLINE	0x01
#define CHR_BLINK	0x80
#define CHR_F_BLACK	0x00
#define CHR_F_BLUE	0x01
#define CHR_F_GREEN	0x02
#define CHR_F_CYAN	0x03
#define CHR_F_RED	0x04
#define CHR_F_MAGENTA	0x05
#define CHR_F_BROWN	0x06
#define CHR_F_WHITE	0x07
#define CHR_F_GREY	0x08
#define CHR_F_YELLOW	0x0e
#define CHR_B_BLACK	0x00
#define CHR_B_BLUE	0x10
#define CHR_B_GREEN	0x20
#define CHR_B_CYAN	0x30
#define CHR_B_RED	0x40
#define CHR_B_MAGENTA	0x50
#define CHR_B_BROWN	0x60
#define CHR_B_WHITE	0x70

#define ATR_NORMAL	   0
#define ATR_BOLD	   1
#define ATR_UNDERLINE	   4
#define ATR_BLINK	   5
#define ATR_REVERSE	   7
#define ATR_INVISIBLE	   8
#define ATR_F_BLACK	  30
#define ATR_F_BLUE	  31
#define ATR_F_GREEN	  32
#define ATR_F_CYAN	  33
#define ATR_F_RED	  34
#define ATR_F_MAGENTA	  35
#define ATR_F_YELLOW	  36
#define ATR_F_WHITE	  37
#define ATR_B_BLACK	  40
#define ATR_B_BLUE	  41
#define ATR_B_GREEN	  42
#define ATR_B_CYAN	  43
#define ATR_B_RED	  44
#define ATR_B_MAGENTA	  45
#define ATR_B_YELLOW	  46
#define ATR_B_WHITE	  47
#endif

#define LINE_NONE	0x00
#define LINE_SINGLE	0x01
#define LINE_DOUBLE	0x02
#define LINE_ASCII	0x04
#define LINE_ASINGLE	(LINE_ASCII | LINE_SINGLE)
#define LINE_ADOUBLE	(LINE_ASCII | LINE_DOUBLE)

#define END_SILENT	0x00
#define END_BELL	0x01
#define END_EXIT	0x02

#define MCUR_NONE	  0x0000
#define MCUR_ENABLED	  0x0001
#define MCUR_HIDDEN	  0x0002
#define MCUR_LEFTCLICK	  0x0004
#define MCUR_RIGHTCLICK   0x0008
#define MCUR_DOUBLECLICK  0x0010
#define MCUR_LEFTBUTTON   0x0020
#define MCUR_RIGHTBUTTON  0x0040


/*===line drawing macros===*/
#define LINE_FIRST	  179
#define LINE_LAST	  218

#define LINE_MASK_LEFT	 0x03
#define LINE_MASK_TOP	 0x0c
#define LINE_MASK_RIGHT  0x30
#define LINE_MASK_BOTTOM 0xc0

#define LINE_SHIFT_LEFT     0
#define LINE_SHIFT_TOP	    2
#define LINE_SHIFT_RIGHT    4
#define LINE_SHIFT_BOTTOM   6

#define LINE_MUX_LEFT	    1	/* 3^0 */
#define LINE_MUX_TOP	    3	/* 3^1 */
#define LINE_MUX_RIGHT	    9	/* 3^2 */ 
#define LINE_MUX_BOTTOM    27	/* 3^3 */ 
/*=========================*/


#ifdef __cplusplus
extern "C" {
#endif

/* window.h */
#define win_gettop()	   ((WIN_IDX) ((windows != NULL) ? win_stack[win_st_top] : 0))
#define win_blank()	   ((int) ((windows != NULL) ? scr_blank(), 1 : 0))
#define win_unblank()	   ((int) ((windows != NULL) ? scr_unblank(), 1 : 0))
#define win_savestate()    ((int) ((windows != NULL) ? scr_savestate(), 1 : 0))
#define win_restorestate() ((int) ((windows != NULL) ? scr_restorestate(), 1 : 0))
#define win_bell()	   ((int) ((windows != NULL) ? scr_bell(), 1 : 0))
#define win_getwp(win)	   ((WIN_REC *) (windows + win))
#define win_setwp(win)	   { win_idx = win; wp = win_getwp(win); }

/* window.c */
int	_win_exp _win_fast win_init	    PROTO((int max_windows, byte curtype, byte conemu, byte chrattrib, byte keyemu));
int	_win_exp _win_fast win_deinit	    PROTO((void));
void	_win_exp _win_fast win_idle	    PROTO((void));
int	_win_exp _win_fast win_setidle	    PROTO((void cdecl (*idlefunc)(void)));
WIN_IDX _win_exp _win_fast win_create	    PROTO((byte winleft, byte wintop, byte winright, byte winbottom,
					       byte curtype, byte conemu, byte chrattrib, byte keyemu));
WIN_IDX _win_exp _win_fast win_open	    PROTO((byte winleft, byte wintop, byte winright, byte winbottom,
					       byte curtype, byte conemu, byte chrattrib, byte keyemu));
WIN_IDX _win_exp _win_fast win_boxopen	    PROTO((byte winleft, byte wintop, byte winright, byte winbottom,
					       byte curtype, byte conemu, byte chrattrib, byte keyemu,
					       byte leftline, byte topline, byte rightline, byte bottomline,
					       byte lineattrib, char *wintitle, byte titleattrib));
int	_win_exp _win_fast win_close	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_hide	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_settop	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_setconemu    PROTO((WIN_IDX win, byte conemu));
byte	_win_exp _win_fast win_getconemu    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_setkeyemu    PROTO((WIN_IDX win, byte keyemu));
byte	_win_exp _win_fast win_getkeyemu    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_setcur	    PROTO((WIN_IDX win, byte curtype));
byte	_win_exp _win_fast win_getcur	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_setrange     PROTO((WIN_IDX win, byte left, byte top, byte right, byte bottom));
int	_win_exp _win_fast win_getrange     PROTO((WIN_IDX win, byte *left, byte *top, byte *right, byte *bottom));
int	_win_exp _win_fast win_cls	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_clrbos	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_clreos	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_clrbol	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_clreol	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_scrollup     PROTO((WIN_IDX win, byte lines));
int	_win_exp _win_fast win_scrolldown   PROTO((WIN_IDX win, byte lines));
int	_win_exp _win_fast win_insline	    PROTO((WIN_IDX win, byte lines));
int	_win_exp _win_fast win_delline	    PROTO((WIN_IDX win, byte lines));
int	_win_exp _win_fast win_inschar	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_delchar	    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_setpos	    PROTO((WIN_IDX win, byte horpos, byte verpos));
int	_win_exp _win_fast win_getpos	    PROTO((WIN_IDX win, byte *horpos, byte *verpos));
int	_win_exp _win_fast win_setattrib    PROTO((WIN_IDX win, byte attrib));
byte	_win_exp _win_fast win_getattrib    PROTO((WIN_IDX win));
int	_win_exp _win_fast win_addattrib    PROTO((WIN_IDX win, byte attrib));
int	_win_exp cdecl win_line 	PROTO((WIN_IDX win, byte horbegin, byte verbegin,
					       byte horend, byte verend, byte linetype));
int	_win_exp _win_fast win_fill	    PROTO((WIN_IDX win, char fillc));
int	_win_exp _win_fast win_putc	    PROTO((WIN_IDX win, char c));
int	_win_exp _win_fast win_puts	    PROTO((WIN_IDX win, char *s));
int	_win_exp _win_fast win_xyputs	    PROTO((WIN_IDX win, byte horpos, byte verpos, char *s));

/* wprintf.c */
int	_win_exp cdecl win_printf	PROTO((WIN_IDX win, char *fmt, ...));
int	_win_exp cdecl win_xyprintf	PROTO((WIN_IDX win, byte horpos, byte verpos, char *fmt, ...));

/* winput.c */
word	_win_exp cdecl win_input	PROTO((byte horpos, byte verpos, byte width, char str[],
					       word abortkeys[], word endkeys[], byte endaction,
					       int cdecl (*checkfunc)(char str[])));

/* key_emu.c */
int		 _win_fast win_keyfull	    PROTO((void));	/* NO EXPORT!*/
int	_win_exp _win_fast win_keyputc	    PROTO((word keycode));
int	_win_exp _win_fast win_keyputs	    PROTO((char *s));
int	_win_exp _win_fast win_keypush	    PROTO((word keycode));
void	_win_exp _win_fast win_keyreset     PROTO((void));
void	_win_exp _win_fast win_keypurge     PROTO((void));
word	_win_exp _win_fast win_keyscan	    PROTO((void));
int	_win_exp _win_fast win_keycheck     PROTO((word keycode));
word	_win_exp _win_fast win_keygetc	    PROTO((void));

/* system dependent file (win_pctc.cas for MS-DOS Borland/Turbo C(++)) */
int	_win_exp _win_fast scr_init	    PROTO((void));
void	_win_exp _win_fast scr_deinit	    PROTO((void));
void	_win_exp _win_fast scr_savestate    PROTO((void));
void	_win_exp _win_fast scr_restorestate PROTO((void));
void	_win_exp _win_fast scr_blank	    PROTO((void));
void	_win_exp _win_fast scr_unblank	    PROTO((void));
void	_win_exp _win_fast scr_bell	    PROTO((void));
void	_win_exp _win_fast scr_update	    PROTO((WIN_REC *wp, byte horpos, byte verpos, int len));
void	_win_exp _win_fast scr_move	    PROTO((byte tohor, byte tover, byte fromhor, byte fromver, int len));
void	_win_exp _win_fast scr_scrollup     PROTO((byte left, byte top, byte width,
					       byte depth, byte lines, byte fillattrib));
void	_win_exp _win_fast scr_scrolldown   PROTO((byte left, byte top, byte width,
					       byte depth, byte lines, byte fillattrib));
void	_win_exp _win_fast scr_setcur	    PROTO((void));
void	_win_exp _win_fast scr_setpos	    PROTO((void));
void	_win_exp _win_fast win_setbuf	    PROTO((SCR_PTR dst, word c, int len));
void	_win_exp _win_fast win_movebuf	    PROTO((SCR_PTR dst, SCR_PTR src, int len));
void	_win_exp cdecl win_timeslice	PROTO((void));
word	_win_exp _win_fast win_getvdutype   PROTO((void));
word	_win_exp _win_fast win_getmttype    PROTO((void));
void	_win_exp _win_fast win_setvdudirect PROTO((int direct));
void	_win_exp _win_fast key_fillbuf	    PROTO((void));
int	_win_exp _win_fast mcur_enable	    PROTO((void));
void	_win_exp _win_fast mcur_disable     PROTO((void));
void	_win_exp _win_fast mcur_show	    PROTO((void));
void	_win_exp _win_fast mcur_hide	    PROTO((void));
void	_win_exp _win_fast mcur_reset	    PROTO((void));
void	_win_exp _win_fast mcur_setrange    PROTO((byte left, byte top, byte right, byte bottom));
void	_win_exp _win_fast mcur_getrange    PROTO((byte *left, byte *top, byte *right, byte *bottom));
void	_win_exp _win_fast mcur_setpos	    PROTO((byte horpos, byte verpos));
void	_win_exp _win_fast mcur_getpos	    PROTO((byte *horpos, byte *verpos));
word	_win_exp _win_fast mcur_status	    PROTO((void));
word	_win_exp _win_fast mcur_buttons     PROTO((void));
WIN_IDX _win_exp _win_fast mcur_getwin	    PROTO((void));
void	_win_exp _win_fast mcur_getwinpos   PROTO((byte *horpos, byte *verpos));
int	_win_exp _win_fast mcur_getcurpos   PROTO((byte *horpos, byte *verpos));

/* con_emu.c */
void	_win_exp _win_fast con_puts	    PROTO((char *s));

#ifdef	__cplusplus
}
#endif


#ifdef __WINDOW_MAIN_
byte	    _win_exp win_maxhor,	/* hor max no. chars on screen	     */
	    _win_exp win_maxver;	/* ver max no. chars on screen	     */
int	    _win_exp num_windows;	/* number of 'windows' in array      */
WIN_REC   * _win_exp windows = NULL;	/* information array of all windows  */
WIN_REC   * _win_exp wp;		/* internal: current window pointer  */
WIN_IDX     _win_exp win_idx;		/* internal: current window index    */
WIN_IDX   * _win_exp win_stack; 	/* win stack (last is top on screen) */
int	    _win_exp win_st_top;	/* last entry on stack (top screen)  */
WIN_IDX   * _win_exp win_tpl;		/* window/screen template buffer     */
WIN_MOUSE * _win_exp win_mouse; 	/* mouse cursor information struct   */
#else
extern	byte	    win_maxhor,
		    win_maxver;
extern	int	    num_windows;
extern	WIN_REC   * windows;
extern	WIN_REC   * wp;
extern	WIN_IDX     win_idx;
extern	WIN_IDX   * win_stack;
extern	int	    win_st_top;
extern	WIN_IDX   * win_tpl;
extern	WIN_MOUSE * win_mouse;

extern byte	    line_ascii[],      /* IBM line drawing paths    */
		    line_table[],      /* IBM line drawing chars    */
		    line_ascii2[],     /* ASCII line drawing paths  */
		    line_table2[];     /* ASCII line drawing chars  */
extern char	    line_set2[];       /* ASCII line drawing set    */
#endif


#include "win_menu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* win_menu.c */
word	_win_exp _win_fast win_menu   PROTO((WIN_MENU *mp, WIN_HOTKEYS *kp));
word	_win_exp _win_fast win_select PROTO((WIN_SELECT *sp));

#ifdef	__cplusplus
}
#endif

#undef PROTO
#endif/*__WINDOW_DEF_*/


/* end of window.h */
