# AGLWIN video windows manager
# Copyright (C) 1990-2001 Arjen G. Lentz
#
# This file is part of AGLWIN.
# AGLWIN is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


#
OBJDIR = OS2WAT
#
COBJS = $(OBJDIR)\window.obj $(OBJDIR)\wprintf.obj $(OBJDIR)\winput.obj \
	$(OBJDIR)\con_emu.obj $(OBJDIR)\key_emu.obj $(OBJDIR)\win_menu.obj \
	$(OBJDIR)\win_os2.obj
#
aglwin2w.lib: $(COBJS) AGLWIN2.DEF
	wcl386 -bd $(COBJS) AGLWIN2.DEF
#
$(COBJS): $*.c window.h win_keys.h win_menu.h
	@md $(OBJDIR) >nul
	wcl386 -c -mf -oxt -fo=$(OBJDIR)\$*.c -d__PROTO__ -d__OS2__ $*.c

# end of makefile
