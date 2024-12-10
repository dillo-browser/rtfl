#ifndef __COMMON_LINES_HH__
#define __COMMON_LINES_HH__

#include "lout/object.hh"
#include "lout/container.hh"

namespace rtfl {

namespace tools {

class LinesSource;

class LinesSink: public lout::object::Object
{
public:
   virtual void setLinesSource (LinesSource *source) = 0;
   virtual void processLine (char *line) = 0;
   virtual void timeout (int type) = 0;
   virtual void finish () = 0;
};

   
class LinesSource: public lout::object::Object
{
public:
   virtual void setup (LinesSink *sink) = 0;
   virtual void addTimeout (double secs, int type) = 0;
   virtual void removeTimeout (int type) = 0;
};


class LinesSourceSequence: public LinesSource
{
private:
   class VirtualSink: public LinesSink
   {
   public:
      LinesSourceSequence *sequence;

      VirtualSink ();
      void setLinesSource (LinesSource *source);
      void processLine (char *line);
      void timeout (int type);
      void finish ();
   };

   VirtualSink virtualSink;
   LinesSink *sink;
   lout::container::typed::List<LinesSource> *sources;
   bool setupCalled;
   lout::container::typed::Iterator<LinesSource> iterator;

public:
   LinesSourceSequence (bool ownerOfSources);
   ~LinesSourceSequence ();
   void add (LinesSource *source);
   void setup (LinesSink *sink);
   void addTimeout (double secs, int type);
   void removeTimeout (int type);
};


class FileLinesSource: public LinesSource
{
private:
   enum { MAX_LINE_SIZE = 1000 };

   tools::LinesSink *sink;
   char buf[MAX_LINE_SIZE + 1];
   int bufPos;
   bool completeLine;

protected:
   FileLinesSource ();
   
   int processInput (int fd);
   inline void setSink (LinesSink *sink) {
      this->sink = sink; sink->setLinesSource (this); }      
   inline LinesSink *getSink () { return sink; }
};


class BlockingLinesSource: public FileLinesSource
{
private:
   class TimeoutInfo: public lout::object::Object
   {
   private:
      long time;
      int type;

   public:
      TimeoutInfo (long time, int type);
      bool equals(Object *other);
      int hashValue();

      inline long getTime () { return time; }
      inline int getType () { return type; }
   };
   
   int fd;
   lout::container::typed::HashSet<TimeoutInfo> *timeoutInfos;

   long getCurrentTime ();
   TimeoutInfo *getNextTimeoutInfo ();
   void processTimeouts ();

public:
   BlockingLinesSource (int fd);
   ~BlockingLinesSource ();
   void setup (LinesSink *sink);
   void addTimeout (double secs, int type);
   void removeTimeout (int type);
};


} // namespace tools

} // namespace rtfl

#endif // __COMMON_LINES_HH__
