# Notes about libraries: "librtfl-tools.a" contains everything not
# depending on FLTK, which can so be used in command line tools;
# "librtfl-common.a" depens on FLTK, and also on the former.

AM_CPPFLAGS = \
	-I$(top_srcdir)

noinst_LIBRARIES = librtfl-common.a librtfl-tools.a

bin_PROGRAMS = rtfl-findrepeat rtfl-tee

librtfl_common_a_SOURCES = \
	about.hh \
	about.cc \
	fltk_lines.hh \
	fltk_lines.cc

librtfl_tools_a_SOURCES = \
	lines.hh \
	lines.cc \
	parser.hh \
	parser.cc \
	tools.hh \
	tools.cc

rtfl_findrepeat_SOURCES = rtfl_findrepeat.cc

rtfl_findrepeat_LDADD = librtfl-tools.a	../lout/liblout.a

rtfl_tee_SOURCES = rtfl_tee.c
