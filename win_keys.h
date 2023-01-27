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


/* win_keys.h - keyboard mapping defines */
#ifndef __WIN_KEYS_DEF_
#define __WIN_KEYS_DEF_

/* In enhanced mode, grey keys have a lower byte of E0, removed by our code  */

#define No_Key		      0 

#define BS		  0x008
#define Alt_BS		  0x10E 	/*	0E00 */
#define Ctrl_BS 	  0x07F

#define Tab		  0x009
#define Alt_Tab 	  0x1A5 	/*	A500 */
#define Shift_Tab	  0x10F
#define Ctrl_Tab	  0x194 	/*	9400 */

#define Enter		  0x00D
#define Alt_Enter	  0x11C 	/*	1C00 */
#define Ctrl_Enter	  0x00A
#define Alt_Num_Enter	  0x1A6 	/*	A600 */

#define Space		  0x020

#define Esc		  0x01B
#define Alt_Esc 	  0x101 	/*	0100 */

#define Ctrl_PrtSc	  0x172 	/* 7200      */

#define Key_45		  0x05C 	/* 565C 565C */
#define Shift_Key_45	  0x07C 	/* 567C      */

#define Ctrl_A		  0x001
#define Ctrl_B		  0x002
#define Ctrl_C		  0x003
#define Ctrl_D		  0x004
#define Ctrl_E		  0x005
#define Ctrl_F		  0x006
#define Ctrl_G		  0x007
#define Ctrl_H		  0x008
#define Ctrl_I		  0x009
#define Ctrl_J		  0x00A
#define Ctrl_K		  0x00B
#define Ctrl_L		  0x00C
#define Ctrl_M		  0x00D
#define Ctrl_N		  0x00E
#define Ctrl_O		  0x00F
#define Ctrl_P		  0x010
#define Ctrl_Q		  0x011
#define Ctrl_R		  0x012
#define Ctrl_S		  0x013
#define Ctrl_T		  0x014
#define Ctrl_U		  0x015
#define Ctrl_V		  0x016
#define Ctrl_W		  0x017
#define Ctrl_X		  0x018
#define Ctrl_Y		  0x019
#define Ctrl_Z		  0x01A

#define MKEY_LEFTMOVE	  0x1D0
#define MKEY_UPMOVE	  0x1D1
#define MKEY_RIGHTMOVE	  0x1D2
#define MKEY_DOWNMOVE	  0x1D3
#define MKEY_LEFTBUTTON   0x1D4
#define MKEY_RIGHTBUTTON  0x1D5
#define MKEY_CURBUTTON	  0x1D6
 
#define Alt_0		  0x181
#define Alt_1		  0x178
#define Alt_2		  0x179
#define Alt_3		  0x17A
#define Alt_4		  0x17B
#define Alt_5		  0x17C
#define Alt_6		  0x17D
#define Alt_7		  0x17E
#define Alt_8		  0x17F
#define Alt_9		  0x180
#define Alt_A		  0x11E
#define Alt_B		  0x130
#define Alt_C		  0x12E
#define Alt_D		  0x120
#define Alt_E		  0x112
#define Alt_F		  0x121
#define Alt_G		  0x122
#define Alt_H		  0x123
#define Alt_I		  0x117
#define Alt_J		  0x124
#define Alt_K		  0x125
#define Alt_L		  0x126
#define Alt_M		  0x132
#define Alt_N		  0x131
#define Alt_O		  0x118
#define Alt_P		  0x119
#define Alt_Q		  0x110
#define Alt_R		  0x113
#define Alt_S		  0x11F
#define Alt_T		  0x114
#define Alt_U		  0x116
#define Alt_V		  0x12F
#define Alt_W		  0x111
#define Alt_X		  0x12D
#define Alt_Y		  0x115
#define Alt_Z		  0x12C

#define F1		  0x13B
#define F2		  0x13C
#define F3		  0x13D
#define F4		  0x13E
#define F5		  0x13F
#define F6		  0x140
#define F7		  0x141
#define F8		  0x142
#define F9		  0x143
#define F10		  0x144
#define F11		  0x185 	/*	8500 */
#define F12		  0x186 	/*	8600 */
#define Alt_F1		  0x168
#define Alt_F2		  0x169
#define Alt_F3		  0x16A
#define Alt_F4		  0x16B
#define Alt_F5		  0x16C
#define Alt_F6		  0x16D
#define Alt_F7		  0x16E
#define Alt_F8		  0x16F
#define Alt_F9		  0x170
#define Alt_F10 	  0x171
#define Alt_F11 	  0x18B 	/*	8B00 */
#define Alt_F12 	  0x18C 	/*	8C00 */
#define Shift_F1	  0x154
#define Shift_F2	  0x155
#define Shift_F3	  0x156
#define Shift_F4	  0x157
#define Shift_F5	  0x158
#define Shift_F6	  0x159
#define Shift_F7	  0x15A
#define Shift_F8	  0x15B
#define Shift_F9	  0x15C
#define Shift_F10	  0x15D
#define Shift_F11	  0x187 	/*	8700 */
#define Shift_F12	  0x188 	/*	8800 */
#define Ctrl_F1 	  0x15E
#define Ctrl_F2 	  0x15F
#define Ctrl_F3 	  0x160
#define Ctrl_F4 	  0x161
#define Ctrl_F5 	  0x162
#define Ctrl_F6 	  0x163
#define Ctrl_F7 	  0x164
#define Ctrl_F8 	  0x165
#define Ctrl_F9 	  0x166
#define Ctrl_F10	  0x167
#define Ctrl_F11	  0x189 	/*	8900 */
#define Ctrl_F12	  0x18A 	/*	8A00 */

#define Up		  0x148
#define Down		  0x150
#define Left		  0x14B
#define Right		  0x14D
#define Home		  0x147
#define End		  0x14F
#define PgUp		  0x149
#define PgDn		  0x151
#define Ins		  0x152
#define Del		  0x153
#define Alt_Up		  0x198 	/*	9800 */
#define Alt_Down	  0x1A0 	/*	A000 */
#define Alt_Left	  0x19B 	/*	9B00 */
#define Alt_Right	  0x19D 	/*	9D00 */
#define Alt_Home	  0x197 	/*	9700 */
#define Alt_End 	  0x19F 	/*	9F00 */
#define Alt_PgUp	  0x199 	/*	9900 */
#define Alt_PgDn	  0x1A1 	/*	A100 */
#define Alt_Ins 	  0x1A2 	/*	A200 */
#define Alt_Del 	  0x1A3 	/*	A300 */
#define Ctrl_Up 	  0x18D 	/*	8DE0 */
#define Ctrl_Down	  0x191 	/*	91E0 */
#define Ctrl_Left	  0x173
#define Ctrl_Right	  0x174
#define Ctrl_Home	  0x177
#define Ctrl_End	  0x175
#define Ctrl_PgUp	  0x184
#define Ctrl_PgDn	  0x176
#define Ctrl_Ins	  0x192 	/*	92E0 */
#define Ctrl_Del	  0x193 	/*	93E0 */

#define Centre		  0x14C 	/*	4C00 */
#define Ctrl_Centre	  0x1F8 	/*	8F00 */

#define Alt_Asterisk	  0x137 	/*	3700 */
#define Alt_Minus	  0x14A 	/*	4A00 */
#define Alt_Plus	  0x14E 	/*	4E00 */
#define Alt_Slash	  0x1A4 	/*	A400 */
#define Ctrl_Asterisk	  0x196 	/*	9600 */
#define Ctrl_Minus	  0x18E 	/*	8E00 */
#define Ctrl_Plus	  0x190 	/*	9000 */
#define Ctrl_Slash	  0x195 	/*	9500 */


#endif/*__WIN_KEYS_DEF_*/


/* end of win_keys.h */
