#ifndef __COMMON_FLTK_LINES_HH__
#define __COMMON_FLTK_LINES_HH__

#include "lines.hh"

namespace rtfl {

namespace common {

class FltkLinesSource: public tools::FileLinesSource
{
   class TimeoutInfo: public lout::object::Object
   {
   private:
      FltkLinesSource *source;
      int type;

   public:
      TimeoutInfo (FltkLinesSource *source, int type);

      inline FltkLinesSource *getSource () { return source; }
      inline int getType () { return type; }
   };

   lout::container::typed::List<TimeoutInfo> *timeoutInfos;

   static void staticProcessInputCallback (int fd, void *data);
   static void timeoutCallback (void *data);
   void processInputCallback (int fd);

public:
   FltkLinesSource ();
   ~FltkLinesSource ();
   
   void setup (tools::LinesSink *sink);
   void addTimeout (double secs, int type);
   void removeTimeout (int type);
};


class FltkDefaultSource: public tools::LinesSourceSequence
{
public:
   FltkDefaultSource ();
};

   
} // namespace common

} // namespace rtfl

#endif // __COMMON_FLTK_LINES_HH__
