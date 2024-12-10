#include "simple_sink.hh"
#include "testtools.hh"

using namespace rtfl::tools;
using namespace rtfl::tests;

int main (int argc, char *argv[])
{
   int fd1 = openPipe ("echo Hello; sleep 2");
   BlockingLinesSource s1 (fd1);
   // Both commands start at the same time, even if fd2 is processed later; thus
   // the "sleep" at the beginning of the second command.
   int fd2 = openPipe ("sleep 2; echo World; sleep 2");
   BlockingLinesSource s2 (fd2);

   LinesSourceSequence lss (false);
   lss.add (&s1);
   lss.add (&s2);
   lss.addTimeout(1, 123);
   lss.addTimeout(3, 124);

   SimpleSink sink;
   lss.setup (&sink);

   return 0;
}
