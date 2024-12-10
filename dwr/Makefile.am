AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DCUR_WORKING_DIR='"@BASE_CUR_WORKING_DIR@/dwr"'

noinst_LIBRARIES =  \
	libDw-rtfl.a

libDw_rtfl_a_SOURCES = \
	box.hh \
	box.cc \
	graph.hh \
	graph.cc \
	hbox.hh \
	hbox.cc \
	hideable.hh \
	hideable.cc \
	label.hh \
	label.cc \
	toggle.hh \
	toggle.cc \
	tools.hh \
	tools.cc \
	vbox.hh \
	vbox.cc

if USE_GRAPH2
libDw_rtfl_a_SOURCES += \
	graph2.hh \
	graph2.cc \
	graph2_iterator.cc
endif
