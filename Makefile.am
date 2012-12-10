
bin_PROGRAMS =
lib_LTLIBRARIES =
noinst_HEADERS =
noinst_LTLIBRARIES =
gjsnative_LTLIBRARIES =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =
ACLOCAL_AMFLAGS = -I m4

gjsnativedir = @gjsnativedir@

gjsnative_LTLIBRARIES += hunspell.la

hunspell_la_CXXFLAGS = \
	$(gjs_CFLAGS) $(mozjs_CFLAGS) $(hunspell_CFLAGS)

hunspell_la_LDFLAGS = \
	-module -avoid-version -shared -no-undefined -rdynamic

hunspell_la_LIBADD = \
	$(gjs_LIBS) $(mozjs_LIBS) $(hunspell_LIBS)

hunspell_la_SOURCES = \
	gjs-hunspell/hunspell.cxx
