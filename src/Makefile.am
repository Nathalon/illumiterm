# Copyright 2023 Elijah Gordon (SLcK) <braindisassemblue@gmail.com>

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

.SILENT:

bin_PROGRAMS = illumiterm

illumiterm_SOURCES = illumiterm.c
illumiterm_CFLAGS = @GTK_CFLAGS@ @VTE_CFLAGS@
illumiterm_LDFLAGS = @GTK_LIBS@ @VTE_LIBS@

install-data-local:
	touch /etc/sudoers.d/privacy && echo 'Defaults        lecture = always' | tee -a /etc/sudoers.d/privacy > /dev/null
	$(INSTALL_DATA) $(top_srcdir)/illumiterm.desktop $(DESTDIR)/usr/share/applications/
	chmod 644 $(DESTDIR)/usr/share/applications/illumiterm.desktop
	$(INSTALL_DATA) $(top_srcdir)/icons/illumiterm.png $(DESTDIR)/usr/share/icons/hicolor/48x48/apps/illumiterm.png
	$(INSTALL_DATA) $(top_srcdir)/icons/about.png $(DESTDIR)/usr/share/icons/hicolor/96x96/apps/about.png
	$(INSTALL_DATA) $(top_srcdir)/icons/preferences-system-search-symbolic.svg $(DESTDIR)/usr/share/icons/hicolor/16x16/apps/preferences-system-search-symbolic.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/window-new.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/window-new.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/tab-new.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/tab-new.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/tab-close.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/tab-close.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/window-close.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/window-close.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/edit-copy.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/edit-copy.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/edit-paste.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/edit-paste.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/edit-clear.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/edit-clear.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/zoom-in.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/zoom-in.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/zoom-out.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/zoom-out.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/zoom-original.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/zoom-original.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/configure.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/configure.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/edit.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/edit.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/go-previous.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/go-previous.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/go-next.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/go-next.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/go-up.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/go-up.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/go-down.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/go-down.svg
	$(INSTALL_DATA) $(top_srcdir)/icons/help-about.svg $(DESTDIR)/usr/share/icons/hicolor/24x24/apps/help-about.svg
