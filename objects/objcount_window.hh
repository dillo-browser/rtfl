#ifndef __OBJECTS_OBJCOUNT_WINDOW_HH__
#define __OBJECTS_OBJCOUNT_WINDOW_HH__

#include <FL/Fl_Window.H>
#include <FL/Fl_Table.H>

#include "lout/object.hh"
#include "lout/container.hh"
#include "lout/misc.hh"

namespace rtfl {

namespace objects {

class ObjCountTable : public Fl_Table
{
private:
   class Class: public lout::object::Comparable
   {
   public:
      char *name;
      int index;
      lout::misc::SimpleVector<int> *count;

      Class (const char *name);
      ~Class ();

      int compareTo(Comparable *other);
      
      void create ();
      void remove ();
      void newSnapshot ();
   };

   class Object: public lout::object::Object
   {
   private:
      static int classSernoGlobal;

      Class *klass;
      int classSerno;
      int refCount;

      ~Object ();

   public:
      Object (Class *klass);
      
      inline void ref () { refCount++; }
      inline void unref () { if (--refCount == 0) delete this; }

      inline Class *getClass () { return klass; }
      void setClass (Class *klass);
      inline int getClassSerno () { return classSerno; }
   };

   class ObjectRef: public lout::object::Object
   {
   public:
      rtfl::objects::ObjCountTable::Object *object;

      ObjectRef (rtfl::objects::ObjCountTable::Object *object);
      ~ObjectRef ();
   };

   lout::container::typed::HashTable<lout::object::String, ObjectRef> *objects;
   lout::container::typed::HashTable<lout::object::String, lout::object::String>
      *identities, *identitiesRev;
   lout::container::typed::HashTable<lout::object::String, Class> *classes;
   lout::container::typed::Vector<Class> *classesList;

   Class *ensureClass (const char *className);
   void insertIdentity (const char *id1, const char *id2);

public:
   ObjCountTable (int x, int y, int width, int height,
                  const char *label = NULL);
   ~ObjCountTable();

   void draw_cell (TableContext context, int row, int col, int x, int y,
                   int width, int height);

   void createObject (const char *id, const char *className);
   void deleteObject (const char *id);
   void registerObject (const char *id);
   void addIdentity (const char *id1, const char *id2);
   void setClassColor (const char *klass, const char *color);
   void newSnapshot ();
   void removeOldestSnapshot ();
};


class ObjCountWindow: public Fl_Window
{
private:
   Fl_Window *aboutWindow;
   ObjCountTable *table;

   static void windowCallback (Fl_Widget *widget, void *data);
   static void quit (Fl_Widget *widget, void *data);
   static void newSnapshot (Fl_Widget *widget, void *data);
   static void removeOldestSnapshot (Fl_Widget *widget, void *data);
   static void about (Fl_Widget *widget, void *data);

public:
   ObjCountWindow (int width, int height, const char *title);
   ~ObjCountWindow ();

   inline ObjCountTable *getTable () { return table; }
};



} // namespace objects

} // namespace rtfl

#endif // __OBJECTS_OBJCOUNT_WINDOW_HH__
