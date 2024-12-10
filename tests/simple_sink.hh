#ifndef __TESTS_SIMPLE_SINK_HH__
#define __TESTS_SIMPLE_SINK_HH__

#include "common/lines.hh"

namespace rtfl {

namespace tests {

class SimpleSink: public rtfl::tools::LinesSink {
private:
   long getCurrentTime ();
   void msg (const char *fmt, ...);

   long startTime;
   
public:
   SimpleSink ();
   
   void setLinesSource (rtfl::tools::LinesSource *source);
   void processLine (char *line);
   void timeout (int type);
   void finish ();
};

} // namespace tests
   
} // namespace rtfl

#endif // __TESTS_SIMPLE_SINK_HH__
