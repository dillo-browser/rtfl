#ifndef __OBJECTS_OBJECTS_WRITER_HH__
#define __OBJECTS_OBJECTS_WRITER_HH__

#include "objects_parser.hh"

namespace rtfl {

namespace objects {

class ObjectsWriter: public ObjectsControllerBase
{
public:
   void objMsg (tools::CommonLineInfo *info, const char *id,
                const char *aspect, int prio, const char *message);
   void objMark (tools::CommonLineInfo *info, const char *id,
                 const char *aspect, int prio, const char *message);
   void objMsgStart (tools::CommonLineInfo *info, const char *id);
   void objMsgEnd (tools::CommonLineInfo *info, const char *id);
   void objEnter (tools::CommonLineInfo *info, const char *id,
                  const char *aspect, int prio, const char *funname,
                  const char *args);
   void objLeave (tools::CommonLineInfo *info, const char *id,
                  const char *vals);
   void objCreate (tools::CommonLineInfo *info, const char *id,
                   const char *klass);
   void objIdent (tools::CommonLineInfo *info, const char *id1,
                  const char *id2);
   void objNoIdent (tools::CommonLineInfo *info);
   void objAssoc (tools::CommonLineInfo *info, const char *parent,
                  const char *child);
   void objSet (tools::CommonLineInfo *info, const char *id, const char *var,
                const char *val);
   void objClassColor (tools::CommonLineInfo *info, const char *klass,
                       const char *color);
   void objObjectColor (tools::CommonLineInfo *info, const char *id,
                        const char *color);
   void objDelete (tools::CommonLineInfo *info, const char *id);
};

} // namespace objects

} // namespace rtfl

#endif // __OBJECTS_OBJVIEW_WRITER_HH__
