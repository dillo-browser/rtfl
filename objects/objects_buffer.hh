#ifndef __OBJECTS_OBJECTS_BUFFER_HH__
#define __OBJECTS_OBJECTS_BUFFER_HH__

#include "objects_parser.hh"
#include "common/tools.hh"

namespace rtfl {

namespace objects {

class ObjectsBuffer: public ObjectsControllerBase
{
private:
   enum CommandType {
      MSG, MARK, MSG_START, MSG_END, ENTER, LEAVE, CREATE, IDENT, NOIDENT,
      ASSOC, SET, CLASS_COLOR, OBJECT_COLOR, DELETE
   };

   class ObjectCommand: public lout::object::Object
   {
      friend class ObjectsBuffer;

   private:
      CommandType type;
      tools::CommonLineInfo info;

      int numArgs;
      struct Arg {
         char type;
         union {
            int d;
            char *s;
         };
      } *args;
      
   public:
      ObjectCommand (CommandType type, tools::CommonLineInfo *info,
                     const char *fmt, ...);
      ~ObjectCommand ();
   };
   
   ObjectsController *successor;
   lout::container::typed::Vector<ObjectCommand> *commandsQueue;
   bool queued;

   void process (ObjectCommand *command);
   void queue (ObjectCommand *command);
   void pass (ObjectCommand *command);

public:   
   ObjectsBuffer (ObjectsController *successor);
   ~ObjectsBuffer ();

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

   void queue ();
   void pass ();
};

} // namespace objects

} // namespace rtfl

#endif // __OBJECTS_OBJECTS_BUFFER_HH__
