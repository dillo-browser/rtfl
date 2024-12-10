AM_CPPFLAGS = \
	-I$(top_srcdir)\
	-DCUR_WORKING_DIR='"@BASE_CUR_WORKING_DIR@/objects"'

noinst_LIBRARIES = librtfl-objects.a

bin_PROGRAMS = rtfl-objbase rtfl-objcount rtfl-objview

rtfl_objbase_SOURCES = rtfl_objbase.cc

rtfl_objbase_LDADD = \
	librtfl-objects.a \
	../common/librtfl-tools.a \
	../lout/liblout.a

librtfl_objects_a_SOURCES = \
	objdelete_controller.hh \
	objdelete_controller.cc \
	objects_buffer.hh \
	objects_buffer.cc \
	objects_parser.hh \
	objects_parser.cc \
	objects_writer.hh \
	objects_writer.cc \
	objident_controller.hh \
	objident_controller.cc

rtfl_objcount_SOURCES = \
	objcount_controller.hh \
	objcount_controller.cc \
	objcount_window.hh \
	objcount_window.cc \
	rtfl_objcount.cc

rtfl_objcount_LDADD = \
	librtfl-objects.a \
	../common/librtfl-common.a \
	../common/librtfl-tools.a \
	../lout/liblout.a \
	@LIBFLTK_LIBS@

rtfl_objview_SOURCES = \
	objview_commands.hh \
	objview_commands.cc \
	objview_graph.hh \
	objview_graph.cc \
	objview_controller.hh \
	objview_controller.cc \
	objview_stacktrace.hh \
	objview_stacktrace.cc \
	objview_window.hh \
	objview_window.cc \
	rtfl_objview.cc

rtfl_objview_LDADD = \
	librtfl-objects.a \
	../common/librtfl-common.a \
	../common/librtfl-tools.a \
	../dwr/libDw-rtfl.a \
	../dw/libDw-fltk.a \
	../dw/libDw-core.a \
	../lout/liblout.a \
	@LIBFLTK_LIBS@

if USE_GRAPH2
rtfl_objview_LDADD += @GRAPHVIZ_LIBS@
endif
