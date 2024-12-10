#include "simple_sink.hh"

using namespace rtfl::tools;
using namespace rtfl::tests;

int main (int argc, char *argv[])
{
   BlockingLinesSource source (0);
   for(int i = 0; i < 5; i++)
      source.addTimeout(1 + i, 123 + i);

   SimpleSink sink;
   source.setup (&sink);

   return 0;
}
