#ifndef __OBJECTS_OBJECTS_PARSER_HH__
#define __OBJECTS_OBJECTS_PARSER_HH__

#include "common/parser.hh"
#include "lout/object.hh"

namespace rtfl {

namespace objects {

class ObjectsSink;

class ObjectsSource
{
public:
   virtual void setObjectsSink (ObjectsSink *sink) = 0;
   virtual void addTimeout (double secs, int type) = 0;
   virtual void removeTimeout (int type) = 0;
};


class ObjectsSink
{
public:
   virtual void setObjectsSource (ObjectsSource *source) = 0;
   virtual void timeout (int type) = 0;
   virtual void finish () = 0;
};


class ObjectsController: public lout::object::Object, public ObjectsSource,
                         public ObjectsSink
{
public:
   virtual void objMsg (tools::CommonLineInfo *info, const char *id,
                        const char *aspect, int prio, const char *message) = 0;
   virtual void objMark (tools::CommonLineInfo *info, const char *id,
                         const char *aspect, int prio, const char *message) = 0;
   virtual void objMsgStart (tools::CommonLineInfo *info, const char *id) = 0;
   virtual void objMsgEnd (tools::CommonLineInfo *info, const char *id) = 0;
   virtual void objEnter (tools::CommonLineInfo *info, const char *id,
                          const char *aspect, int prio, const char *funname,
                          const char *args) = 0;
   virtual void objLeave (tools::CommonLineInfo *info, const char *id,
                          const char *vals) = 0;
   virtual void objCreate (tools::CommonLineInfo *info, const char *id,
                           const char *klass) = 0;
   virtual void objIdent (tools::CommonLineInfo *info, const char *id1,
                          const char *id2) = 0;
   virtual void objNoIdent (tools::CommonLineInfo *info) = 0;
   virtual void objAssoc (tools::CommonLineInfo *info, const char *parent,
                          const char *child) = 0;
   virtual void objSet (tools::CommonLineInfo *info, const char *id,
                        const char *var, const char *val) = 0;
   virtual void objClassColor (tools::CommonLineInfo *info, const char *klass,
                               const char *color) = 0;
   virtual void objObjectColor (tools::CommonLineInfo *info, const char *id,
                                const char *color) = 0;
   virtual void objDelete (tools::CommonLineInfo *info, const char *id) = 0;
};


class ObjectsControllerBase: public ObjectsController
{
   ObjectsSource *predessor;
   ObjectsSink *successor;

protected:
   virtual void ownTimeout (int type);
   virtual void ownFinish ();
   void addOwnTimeout (double secs, int type);
   void removeOwnTimeout (int type);

public:
   ObjectsControllerBase();

   void setObjectsSink (ObjectsSink *sink);
   void addTimeout (double secs, int type);
   void removeTimeout (int type);
   void setObjectsSource (ObjectsSource *source);
   void timeout (int type);
   void finish ();
};


class ObjectsParser: public tools::Parser, public ObjectsSource
{
private:
   ObjectsController *controller;
   tools::LinesSource *source;
   
protected:
   void processCommand (tools::CommonLineInfo *info, char *cmd, char *args);
   void processVCommand (tools::CommonLineInfo *info, const char *module,
                         int majorVersion, int minorVersion, const char *cmd,
                         char **args);

public:
   ObjectsParser (ObjectsController *controller);
   ~ObjectsParser ();

   void setLinesSource (tools::LinesSource *source);
   void timeout (int type);
   void finish ();

   void setObjectsSink (ObjectsSink *sink);
   void addTimeout (double secs, int type);
   void removeTimeout (int type);
};

} // namespace objects

} // namespace rtfl

#endif // __OBJECTS_OBJVIEW_PARSER_HH__
