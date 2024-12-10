AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DCUR_WORKING_DIR='"@BASE_CUR_WORKING_DIR@/tests"'

noinst_PROGRAMS = \
	rtfl-cat \
	rtfl-trickle \
	test-pipes-1 \
	test-select-1 \
	test-version-cmp \
        test-fltk-1 \
        test-fltk-2 \
        test-rtfl-objects-1-without-rtfl \
        test-rtfl-objects-1-with-rtfl \
        test-rtfl-objects-2-without-rtfl \
        test-rtfl-objects-2-with-rtfl \
        test-rtfl-objects-3-without-rtfl \
        test-rtfl-objects-3-with-rtfl \
        test-rtfl-stats-1-without-rtfl \
        test-rtfl-stats-1-with-rtfl \
        test-tools-1 \
	test-tools-2 \
	test-tools-3 \
	test-tools-4 \
	test-tools-5 \
	test-tools-6 \
        test-widgets-1 \
        test-widgets-2 \
        test-widgets-3 \
	test-widget-b-splines

if HAS_GRAPHVIZ
noinst_PROGRAMS += \
        test-graphviz-1
endif

rtfl_cat_SOURCES = rtfl_cat.c

rtfl_trickle_SOURCES = rtfl_trickle.c

test_pipes_1_SOURCES = test_pipes_1.c

test_select_1_SOURCES = test_select_1.c

test_version_cmp_SOURCES = test_version_cmp.c

test_fltk_1_SOURCES = test_fltk_1.cc
test_fltk_1_LDADD = @LIBFLTK_LIBS@

test_fltk_2_SOURCES = test_fltk_2.cc
test_fltk_2_LDADD = @LIBFLTK_LIBS@

test_graphviz_1_SOURCES = test_graphviz_1.c
test_graphviz_1_LDADD = @GRAPHVIZ_LIBS@

test_rtfl_objects_1_without_rtfl_SOURCES = test_rtfl_objects_1.cc
test_rtfl_objects_1_without_rtfl_LDADD = ../lout/liblout.a
test_rtfl_objects_1_with_rtfl_SOURCES = test_rtfl_objects_1_with_rtfl.cc 
test_rtfl_objects_1_with_rtfl_LDADD = ../lout/liblout.a

test_rtfl_objects_2_without_rtfl_SOURCES = test_rtfl_objects_2.cc
test_rtfl_objects_2_with_rtfl_SOURCES = test_rtfl_objects_2_with_rtfl.cc

test_rtfl_objects_3_without_rtfl_SOURCES = test_rtfl_objects_3.cc
test_rtfl_objects_3_with_rtfl_SOURCES = test_rtfl_objects_3_with_rtfl.cc

test_rtfl_stats_1_without_rtfl_SOURCES = test_rtfl_stats_1.cc
test_rtfl_stats_1_without_rtfl_LDADD = ../lout/liblout.a
test_rtfl_stats_1_with_rtfl_SOURCES = test_rtfl_stats_1_with_rtfl.cc 
test_rtfl_stats_1_with_rtfl_LDADD = ../lout/liblout.a

test_tools_1_SOURCES = test_tools_1.cc
test_tools_1_LDADD =  \
        ../common/librtfl-tools.a \
        ../lout/liblout.a

test_tools_2_SOURCES = test_tools_2.cc simple_sink.hh simple_sink.cc \
	testtools.hh testtools.cc
test_tools_2_LDADD =  \
        ../common/librtfl-tools.a \
        ../lout/liblout.a

test_tools_3_SOURCES = test_tools_3.cc  simple_sink.hh simple_sink.cc
test_tools_3_LDADD =  \
        ../common/librtfl-tools.a \
        ../lout/liblout.a

test_tools_4_SOURCES = test_tools_4.cc simple_sink.hh simple_sink.cc \
	testtools.hh testtools.cc
test_tools_4_LDADD =  \
        ../common/librtfl-tools.a \
        ../lout/liblout.a

test_tools_5_SOURCES = test_tools_5.cc simple_sink.hh simple_sink.cc \
	testtools.hh testtools.cc
test_tools_5_LDADD =  \
        ../common/librtfl-tools.a \
        ../lout/liblout.a

test_tools_6_SOURCES = test_tools_6.cc simple_sink.hh simple_sink.cc \
	testtools.hh testtools.cc
test_tools_6_LDADD =  \
        ../common/librtfl-tools.a \
        ../lout/liblout.a

test_widgets_1_SOURCES = test_widgets_1.cc
test_widgets_1_LDADD =  \
        ../dwr/libDw-rtfl.a \
        ../dw/libDw-fltk.a \
        ../dw/libDw-core.a \
        ../lout/liblout.a \
        @LIBFLTK_LIBS@

test_widgets_2_SOURCES = test_widgets_2.cc
test_widgets_2_LDADD =  \
        ../dwr/libDw-rtfl.a \
        ../dw/libDw-fltk.a \
        ../dw/libDw-core.a \
        ../lout/liblout.a \
        @LIBFLTK_LIBS@

test_widgets_3_SOURCES = test_widgets_3.cc
test_widgets_3_LDADD =  \
        ../dwr/libDw-rtfl.a \
        ../dw/libDw-fltk.a \
        ../dw/libDw-core.a \
        ../lout/liblout.a \
        @LIBFLTK_LIBS@

test_widget_b_splines_SOURCES = test_widget_b_splines.cc
test_widget_b_splines_LDADD =  \
        ../dwr/libDw-rtfl.a \
        ../dw/libDw-fltk.a \
        ../dw/libDw-core.a \
        ../lout/liblout.a \
        @LIBFLTK_LIBS@

