#ifndef __OBJECTS_OBJIDENT_CONTROLLER_HH__
#define __OBJECTS_OBJIDENT_CONTROLLER_HH__

#include "objects_parser.hh"
#include "objects_buffer.hh"
#include "common/tools.hh"

namespace rtfl {

namespace objects {

/**
 * \ident Filter Controller for handling `obj-ident`
 * 
 * Motivation
 * ----------
 * When using multiple inheritance in C++, the values for `this` are not
 * identical in all contexts. Consider a class `C`, with `A` and `B` as base
 * classes. The following code
 * 
 *     C *c = new C();
 *     A *a = (A*)c;
 *     B *b = (B*)c;
 *     printf("a = %p\n", a);
 *     printf("b = %p\n", b);
 *     printf("c = %p\n", c);
 * 
 * will output something like this:
 * 
 *     a = 0x1000
 *     b = 0x1010
 *     c = 0x1000
 * 
 * (Notice the different value of `b`.) The value of `b` will differ from `a` by
 * `sizeof(A)`.
 * 
 * For this reason, the tested program has to declare all values as identical,
 * in the following way:
 * 
 *     [rtfl-obj-1.0]...:create:0x1000:A
 *     [rtfl-obj-1.0]...:create:0x1010:B
 *     [rtfl-obj-1.0]...:ident:0x1000:0x1000
 *     [rtfl-obj-1.0]...:ident:0x1000:0x1010
 *     [rtfl-obj-1.0]...:create:0x1000:C
 * 
 * The first `ident` line declares `c` (the newly created instance of `C` as
 * identical to `a` (`(A*)c`), the second declares it identical to `b`
 * (`(B*)c`).
 * 
 * General Approach
 * ----------------
 * Handling `obj-ident` is kept out of the specific implementations of
 * `ObjectsController` and implemented in a filter, `ObjIdentController`, which
 * will translate all identities to actually identical identities; the output of
 * `ObjIdentController` would, in the example above, be:
 * 
 *     [rtfl-obj-1.0]...:create:0x1000:A
 *     [rtfl-obj-1.0]...:create:0x1000:B
 *     [rtfl-obj-1.0]...:create:0x1000:C
 * 
 * Details
 * -------
 * The general problem is that some messages have to be changed (especially
 * `...:create:0x1010:B` has to be changed to `...:create:0x1000:B`) before
 * `obj-ident` is read. Even worse, it is not certain that `obj-ident` will
 * follow at all: the second `obj-create` may actually define a _different_
 * object. For this reason, starting with `obj-create`, all messages are held
 * back until something happens that makes it certain that _no_ related
 * `obj-ident` will follow.
 * 
 * Notice that a functioning implementation is possible even if the second
 * condition is incompletely defined and implemented.
 *
 * (For more complex class hierarchies, it is not sufficient to react on
 * `obj-ident` itself, since another `obj-ident` may follow, dealing with the
 * same identities.)
 * 
 * When is it certain that no related `obj-ident` will follow?
 * 
 * 1. When a method is left (`obj-leave`), in which the objects have been
 *    created.
 * 2. At the end of the stream.
 * 3. (Not an exact condition, but a compromise:) After a timeout of a couple of
 *    seconds; after some time, it can be assumed that the construction of
 *    objects is over.
 *
 * (May be extended. See rtfl::objects::ObjIdentController::objLeave,
 * rtfl::objects::ObjIdentController::ownTimeout, and
 * rtfl::objects::ObjIdentController::ownFinish.
 *
 * Nice to have
 * ------------
 * - Make timeouts configurable, also whether a timeout is triggered after a
 *   certain time after "obj-create" or a certain time after no commands (latter
 *   is currently implemented).
 */
class ObjIdentController: public ObjectsControllerBase
{
private:
   class PostController: public ObjectsControllerBase
   {
      tools::EquivalenceRelation *identities;
      ObjectsController *successor;

      const char *mapId (const char *id);

   public:
      PostController (ObjectsController *successor);
      ~PostController ();
      
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

      void addIdentity (const char *id1, const char *id2);
   };

   enum { PASS = 0 };
   
   ObjectsBuffer *buffer;
   PostController *postController;
   bool noIdent;

   int stackDepth;
   bool createPending;
   int minCreateStackDepth;

   void queue ();
   void pass ();
   
protected:
   void ownTimeout (int type);
   void ownFinish ();

public:   
   ObjIdentController (ObjectsController *successor);
   ~ObjIdentController ();

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

#endif // __OBJECTS_OBJIDENT_CONTROLLER_HH__
