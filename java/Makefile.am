AM_CFLAGS = -Wall $(JAVA_CFLAGS)

JAVA = $(JAVA_HOME)/jre/bin/java
JAVAC = $(JAVA_HOME)/bin/javac

lib_LTLIBRARIES = librtfl-jvm-ti.la

librtfl_jvm_ti_la_SOURCES = \
	main.c \
	class.h \
	class.c \
	method.h \
	method.c \
	field.h \
	field.c \
	config.h \
	config.c \
	misc.h \
	misc.c

EXTRA_DIST = README Hello.java TestRtflObjects1.java

# Run tests without installation.
LIBPATH=./.libs

run-hello: $(LIBPATH)/librtfl-jvm-ti.so rtfl/Hello.class
	LD_LIBRARY_PATH=$(LIBPATH) $(JAVA) -agentlib:rtfl-jvm-ti  rtfl.Hello

run-test-rtfl-objects-1: $(LIBPATH)/librtfl-jvm-ti.so rtfl/TestRtflObjects1.class
	LD_LIBRARY_PATH=$(LIBPATH) $(JAVA) -agentlib:rtfl-jvm-ti  rtfl.TestRtflObjects1

run-test-rtfl-objects-2: $(LIBPATH)/librtfl-jvm-ti.so rtfl/TestRtflObjects2.class
	LD_LIBRARY_PATH=$(LIBPATH) $(JAVA) -agentlib:rtfl-jvm-ti  rtfl.TestRtflObjects2

rtfl/Hello.class: Hello.java
	$(JAVAC) -g -d . Hello.java

rtfl/TestRtflObjects1.class: TestRtflObjects1.java
	$(JAVAC) -g -d . TestRtflObjects1.java

rtfl/TestRtflObjects2.class: TestRtflObjects2.java
	$(JAVAC) -g -d . TestRtflObjects2.java

clean-local:
	find -name "*.class" | xargs rm -f

