#include "simple_sink.hh"
#include "testtools.hh"

using namespace rtfl::tools;
using namespace rtfl::tests;

class NotSoSimpleSink: public SimpleSink
{
private:
   LinesSource *source;
   
public:
   NotSoSimpleSink (LinesSource *source);
   void processLine (char *line);
};

NotSoSimpleSink::NotSoSimpleSink (LinesSource *source)
{
   this->source = source;
}

void NotSoSimpleSink::processLine (char *line)
{
   SimpleSink::processLine (line);
   if (strcmp (line, "create") == 0)
      source->addTimeout (2, 0);
}

int main (int argc, char *argv[])
{
   int fd = openPipe ("echo create; echo msg; sleep 5");
   BlockingLinesSource source (fd);
   NotSoSimpleSink sink (&source);
   source.setup (&sink);

   return 0;
}
