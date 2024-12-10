#include "simple_sink.hh"
#include "testtools.hh"

using namespace rtfl::tools;
using namespace rtfl::tests;

// Test LinesSourceSequence: make sure that LinesSourceSequence deals correctly
// with timeouts.
int main (int argc, char *argv[])
{
   int fd1 = openPipe ("sleep 5");
   BlockingLinesSource s1 (fd1);
   // Both commands start at the same time, even if fd2 is processed later; so
   // it takes 10, not 15 secs totally.
   int fd2 = openPipe ("sleep 10");
   BlockingLinesSource s2 (fd2);

   LinesSourceSequence lss (false);
   lss.add (&s1);
   lss.add (&s2);

   for (int i = 2; i <= 20; i += 2)
      lss.addTimeout(i, i);

   SimpleSink sink;
   lss.setup (&sink);

   return 0;
}
