#! /bin/sh

libtoolize \
&& autoheader \
&& aclocal -I m4 \
&& automake --gnu --add-missing \
&& autoconf
