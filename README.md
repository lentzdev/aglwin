# AGLWIN video windows manager
Copyright (C) 1990-2001 Arjen G. Lentz

This file is part of AGLWIN.
AGLWIN is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.



Hi everybody, this is Arjen's windows manager with keyboard/mouse functions.
It's a multi-platform text windowing library used in Xenia Mailer, Xenia Edit,
and other programs. It will compile under DOS and OS/2, and can use direct
screen access, BIOS calls and VideoFOSSIL.

There's some snazzy stuff, like VT100,VT52 and Avatar video/keyboard emulation.
Each window has its own virtual screen, which is mapped onto the physical with
a template (win_tpl) specifying which window is on top for each character.
See window.h for variable references.



Unzip this archive into a aglwin\ dir under your (Xenia) development root.

If you're using Borland C, check TURBOC.CFG and TLINK.CFG in bc\bin\
TURBO.CFG should have lines like
	-Iu:\bc\include
	-Iu:\bc\include\sys
	-Iu:\bc\aglwin
	-Lu:\bc\lib
	-Lu:\bc\aglwin
TLINK.CFG should contain something like
	/Lu:\bc\lib;u:\bc\aglwin;


In the aglwin\ directory, you will find the following stuff:

- README.md     This file
- LICENSE.md    The GNU General Public License

- MAKELIB.BAT   Call makefile for aglwin_s.lib or aglwin_l.lib (Xenia uses large)
- MAKEFILE.DOS  Generate aglwin library using Borland C and Turbo/Microsoft Link.
Also DOS dependent win_pctc.cas, mouse.h, mouse.cas, far.h, far.c, key.asm

The stuff is clean for BC 3.1, you may be able to use BC 5 which is now free
for download from borland.com, but this has not been tested.
The make utility used is NDMAKE 4.3 or 4.5, which is very unix-alike; don't be
using the crappy Borland make tool unless you want to rewrite the makefile.

- win_nt.c      Unfinished for win32 port, works but not stable.

- win_os2.c     OS/2 dependencies
- aglwin2.def   OS/2 DLL defs
- Using BC/2 1.0 you should get a clean compile; create a aglwin2.dll

- WAT.CMD       Watcom C alternative compile script for OS/2 instead of BC/2 1.0
- WATL.CMD      Watcom C DLL link script for OS/2 to follow WATC.CMD

- test1.c       Test: keyboard input
- test2.c       Test: line drawing

- window.h      Main include file
- 2types.h      Useful defs, sometimes used in other progs without aglwin

- window.c      Main file
- wprint.c      Print routines
- winput.c      Input routnes
- win_menu.c    Menu and select windows
- win_menu.h    Definitions for win_menu.c
- con_emu.c     Console emulation routines
- key_emu.c     Keyboard emulation routines
- win_keys.h    Keyboard definitions


The only thing you should need to do for a port is work on win_pctc.cas
or perhaps easier win_os2.c, stripping out a whole lot of irrelevant stuff,
then doing either term or X. It's pretty self-explanatory.

You may even want to consider making a pipe or TCP port I/O mechanism, that
would be a good and simple start for making Xenia a daemon process. You could
have a single aglwin monitor utility to keep an eye on all your Xenia tasks.


Note: obviously now this code is mostly of historical interest, I don't expect
anyone will still be wanting to do a possible port like they did in 2001...


    -- 20 May 2001, updates 27 Jan 2023
    Arjen G. Lentz

