AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DDILLO_LIBDIR='"$(pkglibdir)/"' \
	-DCUR_WORKING_DIR='"@BASE_CUR_WORKING_DIR@/dw"'

noinst_LIBRARIES = \
	libDw-core.a \
	libDw-fltk.a

libDw_core_a_SOURCES = \
	core.hh \
	events.hh \
	findtext.cc \
	findtext.hh \
	imgbuf.hh \
	imgrenderer.hh \
	imgrenderer.cc \
	iterator.cc \
	iterator.hh \
	layout.cc \
	layout.hh \
	platform.hh \
	selection.hh \
	selection.cc \
	style.cc \
	style.hh \
	types.cc \
	types.hh \
	ui.cc \
	ui.hh \
	view.hh \
	widget.cc \
	widget.hh

# "fltkcomplexbutton.cc", "fltkcomplexbutton.hh", "fltkflatview.cc",
# and "fltkflatview.hh" have been removed from libDw-fltk.a.

libDw_fltk_a_SOURCES = \
	fltkcore.hh \
	fltkimgbuf.cc \
	fltkimgbuf.hh \
	fltkmisc.cc \
	fltkmisc.hh \
	fltkplatform.cc \
	fltkplatform.hh \
	fltkpreview.hh \
	fltkpreview.cc \
	fltkui.cc \
	fltkui.hh \
	fltkviewbase.cc \
	fltkviewbase.hh \
	fltkviewport.cc \
	fltkviewport.hh

libDw_fltk_a_CXXFLAGS = @LIBFLTK_CXXFLAGS@

EXTRA_DIST = preview.xbm
