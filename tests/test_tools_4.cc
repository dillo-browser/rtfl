#include "simple_sink.hh"
#include "testtools.hh"

using namespace rtfl::tools;
using namespace rtfl::tests;

int main (int argc, char *argv[])
{
   int fd =
      openPipe ("for i in $(seq 1 10); do echo Hello world $i; sleep 1; done");
   BlockingLinesSource source (fd);
   for(int i = 0; i < 5; i++)
      source.addTimeout(1 + i, 123 + i);

   SimpleSink sink;
   source.setup (&sink);

   return 0;
}
