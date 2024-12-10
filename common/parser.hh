#ifndef __COMMON_PARSER_HH__
#define __COMMON_PARSER_HH__

#include "lines.hh"

namespace rtfl {

namespace tools {

struct CommonLineInfo
{
   char *fileName;
   int lineNo;
   int processId;
   char *completeLine;
};

class Parser: public LinesSink
{
private:
   char **splitEscaped (char *txt);
   void scanSplit (char *txt, int *numParts, char **parts);
   static void unquote (char *txt);
   void freeSplitEscaped (char **parts);

protected:
   char **split (char *txt, int maxNum);
   void freeSplit (char **parts);

   virtual void processCommand (CommonLineInfo *info, char *cmd, char *args)
      = 0;
   virtual void processVCommand (CommonLineInfo *info, const char *module,
                                 int majorVersion, int minorVersion,
                                 const char *cmd, char **args) = 0;

public:
   void setLinesSource (LinesSource *source);
   void processLine (char *line);
   void finish ();
   void timeout (int type);
};

} // namespace common

} // namespace rtfl

#endif // __COMMON_PARSER_HH__
