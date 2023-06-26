#!/bin/sh

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

if [ -e .gitmodules ] && [ -e .git ]; then
    echo "* Updating git submodules"
    git submodule update --init
fi

: "${LIBTOOLIZE:=libtoolize}"
: "${AUTORECONF:=autoreconf}"
: "${ACLOCAL:=aclocal}"
: "${AUTOCONF:=autoconf}"
: "${AUTOHEADER:=autoheader}"
: "${AUTOMAKE:=automake}"

check_command() {
    if ! command -v "$1" > /dev/null 2>&1; then
        echo "* Error: $1 is required to run autogen.sh" >&2
        exit 1
    fi
}

check_command "$LIBTOOLIZE"
check_command "$AUTORECONF"
check_command "$ACLOCAL"
check_command "$AUTOCONF"
check_command "$AUTOHEADER"
check_command "$AUTOMAKE"

echo "* Running autoreconf"
$AUTORECONF -vif || {
    echo "* Error: autoreconf failed. Running autotools manually." >&2
    for dir in "$PWD"/*; do
        if [ ! -d "$dir" ] || [ ! -f "$dir"/configure.ac ] && [ ! -f "$dir"/configure.in ]; then
            continue
        fi
        echo "* Running autotools for $(basename "$dir")"
        (cd "$dir" && $ACLOCAL --force -I m4)
        (cd "$dir" && $AUTOMAKE --add-missing --copy --force-missing)
        (cd "$dir" && autoupdate)  # Run autoupdate here
        (cd "$dir" && $LIBTOOLIZE --copy --force)
        (cd "$dir" && $AUTOCONF --force)
        (cd "$dir" && $AUTOHEADER)
    done
}

echo "* To continue, run ./configure"
