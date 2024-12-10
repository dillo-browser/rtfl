#include "simple_sink.hh"
#include "common/tools.hh"

#include <unistd.h>
#include <sys/time.h>

namespace rtfl {

namespace tests {

using namespace rtfl::tools;

SimpleSink::SimpleSink ()
{
   startTime = getCurrentTime ();
   msg ("<init>");
}

void SimpleSink::setLinesSource (LinesSource *source)
{
   msg ("setLinesSource: souce = %p", source);
}

void SimpleSink::processLine (char *line)
{
   msg ("processLine: %s", line);
}

void SimpleSink::timeout (int type)
{
   msg ("timeout: type = %d", type);
}

void SimpleSink::finish ()
{
   msg ("finish");
}

long SimpleSink::getCurrentTime ()
{
   struct timeval tv;

   if (gettimeofday (&tv, NULL) != 0)
      syserr ("gettimeofday() failed");

   return tv.tv_sec * 1000L + tv.tv_usec / 1000L;
}

void SimpleSink::msg (const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);

   long time = getCurrentTime () - startTime;
   printf ("[SimpleSink] %2ld.%03ld -- ", time / 1000, time % 1000);

   vprintf (fmt, args);
   putchar ('\n');
}

} // namespace tests
   
} // namespace rtfl
