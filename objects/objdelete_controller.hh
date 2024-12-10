#ifndef __OBJECTS_OBJDELETE_CONTROLLER_HH__
#define __OBJECTS_OBJDELETE_CONTROLLER_HH__

#include "objects_parser.hh"
#include "common/tools.hh"

namespace rtfl {

namespace objects {

/**
 * \brief Processes `obj-delete` specially and maps ids of deleted objects to
 *    new ones, if they are reused.
 */
class ObjDeleteController: public ObjectsControllerBase
{
private:
   class ObjInfo: public lout::object::Object
   {
   private:
      int numCreated, numDeleted;
      char *origId, *mappedId;

   public:
      ObjInfo (const char *id);
      ~ObjInfo ();

      void use ();
      void objCreate ();
      void objDelete ();

      inline const char *getMappedId () { return mappedId; }
   };
   
   ObjectsController *successor;
   lout::container::typed::HashTable<lout::object::String, ObjInfo> *objInfos;

   const char *mapId (const char *id);
   bool objInfoCreated (const char *id);
   ObjInfo *getObjInfo (const char *id);
   ObjInfo *ensureObjInfo (const char *id);

public:   
   ObjDeleteController (ObjectsController *successor);
   ~ObjDeleteController ();

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

#endif // __OBJECTS_OBJDELETE_CONTROLLER_HH__
