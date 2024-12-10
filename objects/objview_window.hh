#ifndef __OBJECTS_OBJVIEW_WINDOW_HH__
#define __OBJECTS_OBJVIEW_WINDOW_HH__

#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include "objview_graph.hh"
#include "lout/container.hh"
#include "common/about.hh"

namespace rtfl {

namespace objects {

class ObjViewWindow: public Fl_Window, public ObjViewFilterTool
{
private:
   class Aspect: public lout::object::Comparable
   {
   public:
      ObjViewWindow *window;
      char *name;
      bool set;
      Fl_Menu_Item *menuItem;

      Aspect (ObjViewWindow *window, const char *name, bool set);
      ~Aspect ();

      bool equals(Object *other);
      int compareTo(Comparable *other);
   };

   class Priority: public lout::object::Comparable
   {
   public:
      ObjViewWindow *window;
      int value;
      char valueBuf[6];
      Fl_Menu_Item *menuItem;

      Priority (ObjViewWindow *window, int value);

      bool equals(Object *other);
      int compareTo(Comparable *other);
   };

   bool shown;

   ::dw::core::Layout *layout;
   ObjViewGraph *graph;
   Fl_Window *aboutWindow;
   Fl_Menu_Bar *menu;

   lout::container::typed::HashTable<lout::object::String, Aspect> *aspects;
   lout::container::typed::Vector<lout::object::Integer> *aspectsMenuPositions;
   bool aspectsInitiallySet;

   lout::container::typed::HashTable<lout::object::Integer, Priority>
      *priorities;
   lout::container::typed::Vector<lout::object::Integer>
      *prioritiesMenuPositions;
   int selectedPriority;

   inline Fl_Menu_Item *getCreateMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Command/&Creations"); }

   inline Fl_Menu_Item *getIndentMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Command/&Indentations"); }

   inline Fl_Menu_Item *getMessageMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Command/&Messages"); }

   inline Fl_Menu_Item *getMarkMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Command/M&arks"); }

   inline Fl_Menu_Item *getFunctionMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Command/&Functions"); }

   inline Fl_Menu_Item *getAssocMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Command/A&ssociations"); }
      
   inline Fl_Menu_Item *getAddAttrMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Command/A&ttributes"); }

   inline Fl_Menu_Item *getDeleteMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Command/&Deletions"); }

   inline Fl_Menu_Item *getShowAllAspectsMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Aspects/&Show all"); }

   inline Fl_Menu_Item *getHideAllAspectsMenuItem ()
   { return (Fl_Menu_Item*)menu->find_item("&Aspects/&Hide all"); }

   static void quit (Fl_Widget *widget, void *data);
   static void previous (Fl_Widget *widget, void *data);
   static void next (Fl_Widget *widget, void *data);
   static void viewCode (Fl_Widget *widget, void *data);
   static void hideBefore (Fl_Widget *widget, void *data);
   static void hideAfter (Fl_Widget *widget, void *data);
   static void hideAll (Fl_Widget *widget, void *data);
   static void showBefore (Fl_Widget *widget, void *data);
   static void showAfter (Fl_Widget *widget, void *data);
   static void showAll (Fl_Widget *widget, void *data);
   static void showStackTrace (Fl_Widget *widget, void *data);
   static void switchBetweenRelated (Fl_Widget *widget, void *data);
   static void toggleCommandTypeVisibility (Fl_Widget *widget, void *data);
   static void showAllAspects (Fl_Widget *widget, void *data);
   static void hideAllAspects (Fl_Widget *widget, void *data);
   static void toggleAspect (Fl_Widget *widget, void *data);
   static void setPriority (Fl_Widget *widget, void *data);
   static void jumpToMark (Fl_Widget *widget, void *data);
   static void about (Fl_Widget *widget, void *data);
   static void windowCallback (Fl_Widget *widget, void *data);

   Priority *addPriority (int priority, bool val);

   void showOrHideAllAspects (bool value);

public:
   ObjViewWindow (int width, int height, const char *title);
   ~ObjViewWindow ();

   void show();

   void addAspect (const char *aspect);
   void addPriority (int priority);
   bool isAspectSelected (const char *aspect);
   bool isPrioritySelected (int priority);
   bool isTypeSelected (OVGCommandType type);
   void addMark (OVGAddMarkCommand *markCommand);

   void addAspect (const char *aspect, bool val);
   void setAspectsInitiallySet (bool val);
   void setPriority (int priority);
   void setAnyPriority ();

   inline ObjViewGraph *getObjViewGraph () { return graph; }

   inline void showCreateCommands (bool val) {
      if (val) getCreateMenuItem()->set ();
      else getCreateMenuItem()->clear ();
   }
   
   inline void showIndentCommands (bool val) {
      if (val) getIndentMenuItem()->set ();
      else getIndentMenuItem()->clear ();
   }

   inline void showMessageCommands (bool val) {
      if (val) getMessageMenuItem()->set ();
      else getMessageMenuItem()->clear ();
   }

   inline void showMarkCommands (bool val) {
      if (val) getMarkMenuItem()->set ();
      else getMarkMenuItem()->clear ();
   }

   inline void showFunctionCommands (bool val) {
      if (val) getFunctionMenuItem()->set ();
      else getFunctionMenuItem()->clear ();
   }

   inline void showAssocCommands (bool val) {
      if (val) getAssocMenuItem()->set ();
      else getAssocMenuItem()->clear ();
   }

   inline void showAddAttrCommands (bool val) {
      if (val) getAddAttrMenuItem()->set ();
      else getAddAttrMenuItem()->clear ();
   }

   inline void showDeleteCommands (bool val) {
      if (val) getDeleteMenuItem()->set ();
      else getDeleteMenuItem()->clear ();
   }

   inline void showObjectMessages (bool val) { graph->showObjectMessages(val); }
   inline void showObjectContents (bool val) { graph->showObjectContents(val); }
   inline void setCodeViewer (const char *codeViewer) {
      graph->setCodeViewer (codeViewer); }
};

} // namespace objects

} // namespace rtfl

#endif // __OBJECTS_OBJVIEW_WINDOW_HH__
