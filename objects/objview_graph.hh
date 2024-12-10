#ifndef __OBJECTS_OBJVIEW_GRAPH_HH__
#define __OBJECTS_OBJVIEW_GRAPH_HH__

#include "objview_stacktrace.hh"
#include "objview_commands.hh"
#include "dwr/graph.hh"
#include "dwr/graph2.hh"
#include "dwr/toggle.hh"
#include "dwr/vbox.hh"
#include "dwr/hbox.hh"
#include "dwr/label.hh"
#include "common/tools.hh"

namespace rtfl {

namespace objects {

enum OVGCommandType {
   OVG_COMMAND_CREATE,
   OVG_COMMAND_INDENT,
   OVG_COMMAND_MESSAGE,
   OVG_COMMAND_MARK,
   OVG_COMMAND_FUNCTION,
   OVG_COMMAND_ASSOC,
   OVG_COMMAND_ADD_ATTR,
   OVG_COMMAND_DELETE
};

class ObjViewGraphListener: public ObjViewListener
{
private:
   ObjViewGraph *graph;

public:
   ObjViewGraphListener (ObjViewGraph *graph) { this->graph = graph; }

   void close (ObjViewStacktraceWindow *window);
};


class ObjViewFilterTool
{
public:
   virtual void addAspect (const char *aspect) = 0;
   virtual void addPriority (int priority) = 0;
   virtual bool isAspectSelected (const char *aspect) = 0;
   virtual bool isPrioritySelected (int priority) = 0;
   virtual bool isTypeSelected (OVGCommandType type) = 0;
   virtual void addMark (OVGAddMarkCommand *markCommand) = 0;
};

class OVGAttribute;

class OVGAttributesList: public lout::object::Object
{
private:
   OVGAttributesList *parent;
   int childNoCount, numChildrenShown, childNo;
   lout::misc::BitSet *childrenShown;
   bool shown;

   void checkVisibility ();

protected:
   lout::container::typed::HashTable<lout::object::String,
                                     OVGAttribute> *attributes;

   virtual void show () = 0;
   virtual void hide () = 0;
   
public:
   dw::VBox *vbox;
   dw::Toggle *toggle;

   OVGAttributesList (OVGAttributesList *parent);
   ~OVGAttributesList ();

   inline OVGAttribute *get (lout::object::String *key)
   { return attributes->get (key); }
   virtual int add (lout::object::String *key, OVGAttribute *attribute) = 0;

   void initWidgets (::dw::core::style::Style *widgetStyle, dw::Box *parent,
                     bool showLarge);

   int registerChild ();
   void unregisterChild (int childNo);
   void childShown (int childNo);
   void childHidden (int childNo);
};

class OVGTopAttributes: public OVGAttributesList
{
protected:
   lout::container::typed::Vector<lout::object::String> *sortedList;

   void show ();
   void hide ();

public: 
   OVGTopAttributes ();
   ~OVGTopAttributes ();

   int add (lout::object::String *key, OVGAttribute *attribute);
};

// Hint: the OVGAttributesList'ness refers to the children.
class OVGAttribute: public OVGAttributesList
{
protected:
   void show ();
   void hide ();

public:
   dw::HBox *hbox;
   dw::Label *label;
   char *name;
   
   OVGAttribute (const char *name, OVGAttributesList *parent);
   ~OVGAttribute ();
   
   void initWidgets (::dw::core::style::Style *widgetStyle,
                     dw::Box *parent, bool showLarge, int newPos);

   int add (lout::object::String *key, OVGAttribute *attribute);
};

class ObjViewGraph: public
#if USE_GRAPH2
   dw::Graph2
#else
   dw::Graph
#endif
{
   friend class OVGCommonCommand;
   friend class OVGIncIndentCommand;
   friend class OVGDecIndentCommand;
   friend class OVGEnterCommand;
   friend class OVGLeaveCommand;
   friend class OVGAddMessageCommand;
   friend class OVGAddMarkCommand;
   friend class OVGCreateCommand;
   friend class OVGAddAssocCommand;
   friend class OVGAddAttrCommand;
   friend class OVGDeleteCommand;
   friend class ObjViewGraphListener;

private:
   class CreateRefCommand;

   class GraphObject: public lout::object::Object
   {
      friend class CreateRefCommand;
      friend class ObjViewGraph;
      
   private:
      ObjViewGraph *graph;

   public:
      char *id, *className;
      ::dw::core::style::Style *messageStyle;
      dw::Toggle *node;
      dw::Label *id1, *id2;
      OVGTopAttributes *attributes;
      dw::VBox *messages;
      
      GraphObject (ObjViewGraph *graph, const char *id);
      ~GraphObject ();

   };

   // Inernally added for the first occurence of an identity.
   class CreateRefCommand: public OVGCommonCommand
   {
   protected:
      void doExec ();
      void doUndo ();

   public:
      CreateRefCommand (ObjViewGraph *graph, const char *id,
                        GraphObject *graphObject);
   };

   class Color: public lout::object::Object
   {
   public:
      char *identifier;
      ::dw::core::style::Color *color;
      
      Color (const char *classPattern, const char *color,
                  ::dw::core::Layout *layout);
      ~Color ();
   };

   class ColorComparator: public lout::object::Comparator
   {
   private:
      int specifity (const char *pattern);

   public:
      int compare(Object *o1, Object *o2);
   };

   ObjViewFilterTool *filterTool;

   lout::container::typed::Vector<OVGCommand> *commands;
   lout::container::typed::Vector<OVGCommand> *navigableCommands;
   lout::container::typed::HashTable<lout::object::String,
                                     GraphObject> *objectsById;
   lout::container::typed::Vector<GraphObject> *allObjects;  
   lout::container::typed::Vector<Color> *classColors;
   lout::container::typed::Vector<Color> *objectColors;

   lout::container::typed::Stack<OVGEnterCommand> *enterCommands;
   lout::container::typed::Stack<OVGIncIndentCommand> *startCommands;

   bool objectContents, objectMessages;
   char *codeViewer;
   int navigableCommandsPos, hiddenBefore, hiddenAfter, numVisibleCommands;

   ::dw::core::style::Style *nodeStyle, *noBorderStyle, *topBorderStyle,
        *bottomBorderStyle, *leftBorderStyle;
   bool inDestructor;

   lout::container::typed::List <ObjViewStacktraceWindow> *stacktraceWindows;
   ObjViewGraphListener *stacktraceListener;

   GraphObject *ensureObject (const char *id);
   OVGAttribute *addAttribute (OVGAttributesList *attributesList, char **parts,
                               const char *value, dw::Label **smallLabel,
                               dw::HBox **histBox, dw::Label **indexLabel,
                               dw::Label **histLabel);

   ::dw::core::style::Color *getObjectColor (GraphObject *obj);
   void applyClassOrObjectStyle (GraphObject *obj);
   void applyClassOrObjectStyles ();

   void addMessage (const char *id, const char *message, dw::HBox **mainBox,
                    dw::Label **indexLabel, dw::Label **mainLabel);
   bool incIndent (const char *id);
   bool decIndent (const char *id);
   OVGAttribute *addAttribute (const char *id, const char *name,
                               const char *value, dw::Label **smallLabel,
                               dw::HBox **histBox, dw::Label **indexLabel,
                               dw::Label **histLabel);
   void setClassName (const char *id, const char *className);
   void addAssoc (const char *id1, const char *id2);

   void clearSelection ();
   void unclearSelection ();
   int firstCommand ();
   int lastCommand ();

public:
   ObjViewGraph (ObjViewFilterTool *objectFilterTool);
   ~ObjViewGraph ();
   void initStyles (const char *fontName, int fontSize, int fgColor,
                    int graphBgColor, int objBgColor, int borderThickness,
                    int grraphMargin);

   void addCommand (OVGCommand *command, bool navigable);
   void addIdentity (const char *id1, const char *id2);
   void setClassColor (const char *klass, const char *color);
   void setObjectColor (const char *id, const char *color);
   
   int getObjectColor (const char *id);

   inline void pushEnterCommand (OVGEnterCommand *enterCommand)
   { enterCommands->push (enterCommand); }
   void popEnterCommand ()  { enterCommands->pop (); }
   OVGEnterCommand *getLastEnterCommand () { return enterCommands->getTop (); }

   inline void pushStartCommand (OVGIncIndentCommand *startCommand)
   { startCommands->push (startCommand); }
   void popStartCommand ()  { startCommands->pop (); }
   OVGIncIndentCommand *getLastStartCommand ()
   { return startCommands->getTop (); }

   inline void addCommandMark (OVGAddMarkCommand *markCommand)
   { filterTool->addMark (markCommand); }

   void previousCommand ();
   void nextCommand ();
   void viewCodeOfCommand ();
   void showStackTraceOfCommand ();
   void switchBetweenRelatedCommands ();
   void setCommand (int index);
   void hideBeforeCommand ();
   void hideAfterCommand ();
   void hideAllCommands ();
   void showBeforeCommand ();
   void showAfterCommand ();
   void showAllCommands ();

   inline void showObjectContents (bool val) { objectContents = val; }
   inline void showObjectMessages (bool val) { objectMessages = val; }
   void setCodeViewer (const char *codeViewer);

   void recalculateCommandsVisibility ();
};

} // namespace objects

} // namespace rtfl

#endif // __OBJECTS_OBJVIEW_GRAPH_HH__
