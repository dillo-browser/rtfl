SUBDIRS = lout common dw dwr objects scripts tests doc

if HAS_JAVA
  SUBDIRS += java
endif

ACLOCAL_AMFLAGS=-I m4

# "debug_rtfl.hh" is generated from "debug_rtfl.hh.in". When calling
# "make", the subdirectories are processed before "debug_rtfl.hh" is
# generated, so it may be that you have to generate it explicitly by
# calling "make debug_rtfl.hh".

# For convenience, "debug_rtfl.hh" is included into the distribution.
# Furthermore (mainly because of the problem mentioned above), it is
# also added to the SVN repository.

EXTRA_DIST = debug_rtfl.hh debug_rtfl.hh.in

debug_rtfl.hh: debug_rtfl.hh.in create-debug_rtfl-hh
	./create-debug_rtfl-hh < debug_rtfl.hh.in > debug_rtfl.hh
