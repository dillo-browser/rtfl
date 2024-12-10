#include "simple_sink.hh"
#include "common/tools.hh"

#include <unistd.h>
#include <sys/timeb.h>

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
   struct timeb t;
   if (ftime (&t) == -1)
      syserr ("ftime() failed");
   return t.time * 1000L + t.millitm;
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
