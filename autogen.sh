#!/bin/sh

set -e

builddir="$(pwd)"
srcdir="$(dirname "$0")"

cd "$srcdir"
autoreconf -vfi
intltoolize --copy

cd "$builddir"
[ -z "$NOCONFIGURE" ] && exec "$srcdir"/configure "$@"
