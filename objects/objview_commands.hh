#ifndef __OBJECTS_OBJVIEW_COMMANDS_HH__
#define __OBJECTS_OBJVIEW_COMMANDS_HH__

#include "objview_stacktrace.hh"
#include "dwr/toggle.hh"
#include "dwr/vbox.hh"
#include "dwr/hbox.hh"
#include "dwr/label.hh"

namespace rtfl {

namespace objects {

class ObjViewGraph;
class OVGAttribute;

/*
 * TODO: OVGCommand was originally introduced to provide a undo/redo mechanism
 * needed for handling "obj-ident". Now, after "obj-ident" is handled by
 * ObjIdentController, OVGCommand can be simplified and reduced to navigable
 * commands.
 */
   
class OVGCommand: public lout::object::Object
{
public:
   virtual const char *getFileName () = 0;
   virtual int getLineNo () = 0;
   virtual int getProcessId () = 0;

   virtual void exec (ObjViewGraph *graph) = 0;
   virtual void show (ObjViewGraph *graph) = 0;
   virtual void hide (ObjViewGraph *graph) = 0;
   virtual void scrollTo (ObjViewGraph *graph) = 0;
   virtual void select (ObjViewGraph *graph) = 0;
   virtual void unselect (ObjViewGraph *graph) = 0;
   virtual bool isVisible (ObjViewGraph *graph) = 0;
   virtual bool calcVisibility (ObjViewGraph *graph) = 0;
   virtual int getNavigableCommandsIndex (ObjViewGraph *graph) = 0;
   virtual void setNavigableCommandsIndex (ObjViewGraph *graph,
                                           int navigableCommandsIndex) = 0;
   virtual void setVisibleIndex (ObjViewGraph *graph,
                                 int visibleIndex) = 0;
   virtual ObjViewFunction *getFunction () = 0;
   virtual void setRelatedCommand (OVGCommand *relatedCommand) = 0;
   virtual OVGCommand *getRelatedCommand () = 0;
};


class OVGCommonCommand: public OVGCommand
{
private:
   bool executed, visible;
   ObjViewFunction *function;
   OVGCommand *relatedCommand;

protected:
   class CommandLinkReceiver: public ::dw::core::Layout::LinkReceiver
   {
   private:
      ObjViewGraph *graph;
      OVGCommand *command;
      
   public:
      inline CommandLinkReceiver () { graph = NULL; }
      inline void setData (ObjViewGraph *graph,
                           OVGCommand *command)
      { this->graph = graph; this->command = command; }
      
      bool click (::dw::core::Widget *widget, int link, int img, int x, int y,
                  ::dw::core::EventButton *event);
   };

   char *id;
   ObjViewGraph *graph;
  
   virtual void doExec () = 0;
   virtual void doShow ();
   virtual void doHide ();
   virtual void doScrollTo ();
   virtual void doSelect ();
   virtual void doUnselect ();

public:
   OVGCommonCommand (ObjViewGraph *graph, const char *id,
                     ObjViewFunction *function);
   ~OVGCommonCommand ();

   const char *getFileName ();
   int getLineNo ();
   int getProcessId ();

   void exec (ObjViewGraph *graph);
   void show (ObjViewGraph *graph);
   void hide (ObjViewGraph *graph);
   void scrollTo (ObjViewGraph *graph);
   void select (ObjViewGraph *graph);
   void unselect (ObjViewGraph *graph);
   bool isVisible (ObjViewGraph *graph);
   bool calcVisibility (ObjViewGraph *graph);
   int getNavigableCommandsIndex (ObjViewGraph *graph);
   void setNavigableCommandsIndex (ObjViewGraph *graph,
                                   int navigableCommandsIndex);
   void setVisibleIndex (ObjViewGraph *graph, int visibleIndex);
   ObjViewFunction *getFunction () { return function; }
   void setRelatedCommand (OVGCommand *relatedCommand)
   { this->relatedCommand = relatedCommand; }
   OVGCommand *getRelatedCommand () { return relatedCommand; }
};


class OVGNavigableCommand: public OVGCommonCommand
{
private:
   char *fileName;
   int lineNo;

protected:
   int navigableCommandsIndex;
   dw::HBox *hbox;
   dw::Label *label1, *label2;
   CommandLinkReceiver linkReceiver;

   void doShow ();
   void doHide ();
   void doScrollTo ();
   void doSelect ();
   void doUnselect ();

   void initWidgets ();
   void removeWidgets ();

public:
   OVGNavigableCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                        const char *id, ObjViewFunction *function);
   ~OVGNavigableCommand ();

   const char *getFileName ();
   int getLineNo ();

   int getNavigableCommandsIndex (ObjViewGraph *graph);
   void setNavigableCommandsIndex (ObjViewGraph *graph,
                                   int navigableCommandsIndex);
   void setVisibleIndex (ObjViewGraph *graph, int visibleIndex);
};

class OVGIncIndentCommand: public OVGNavigableCommand
{
private:
   bool success;

protected:
   void doExec ();

public:
   OVGIncIndentCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                        const char *id, ObjViewFunction *function);
   ~OVGIncIndentCommand ();

   bool calcVisibility (ObjViewGraph *graph);
};


class OVGDecIndentCommand: public OVGNavigableCommand
{
private:
   bool success;

protected:
   void doExec ();

public:
   OVGDecIndentCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                        const char *id, OVGIncIndentCommand *startCommand,
                        ObjViewFunction *function);
   ~OVGDecIndentCommand ();

   bool calcVisibility (ObjViewGraph *graph);
};

class OVGEnterCommand: public OVGNavigableCommand, public ObjViewFunction
{
private:
   char *aspect, *funname, *args;
   int prio;
   bool success;
   
protected:
   void doExec ();

public:
   OVGEnterCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                    const char *id, const char *aspect, int prio,
                    const char *funname, const char *args,
                    ObjViewFunction *function);
   ~OVGEnterCommand ();

   bool calcVisibility (ObjViewGraph *graph);

   char *createMessage (const char *c1, const char *c2, const char *c3a,
                        const char *c3b);
   void freeMessage (char *message);

   char *createName ();
   void freeName (char *name);
   int getColor ();
   ObjViewFunction *getParent ();
   bool isSelectable ();
   void select ();
};

class OVGLeaveCommand: public OVGNavigableCommand
{
private:
   OVGEnterCommand *enterCommand;
   char *vals;
   bool success;

protected:
   void doExec ();

public:
   OVGLeaveCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                    const char *id, const char *vals,
                    OVGEnterCommand *enterCommand);
   ~OVGLeaveCommand ();

   bool calcVisibility (ObjViewGraph *graph);
};

class OVGAddMessageCommand: public OVGNavigableCommand
{
private:
   char *aspect, *message;
   int prio;
   
protected:
   void doExec ();

public:
   OVGAddMessageCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                         const char *id, const char *aspect, int prio,
                         const char *message, ObjViewFunction *function);
   ~OVGAddMessageCommand ();

   bool calcVisibility (ObjViewGraph *graph);
};


class OVGAddMarkCommand: public OVGNavigableCommand
{
private:
   char *aspect, *mark;
   int prio;
   
protected:
   void doExec ();

public:
   OVGAddMarkCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                      const char *id, const char *aspect, int prio,
                      const char *mark, ObjViewFunction *function);
   ~OVGAddMarkCommand ();

   bool calcVisibility (ObjViewGraph *graph);

   const char *getMark () { return mark; }
   bool isSelectable ();
   void select ();
};


class OVGCreateCommand: public OVGNavigableCommand
{
private:
   char *className;

protected:
   void doExec ();

public:
   OVGCreateCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                     const char *id, const char *className,
                     ObjViewFunction *function);
   ~OVGCreateCommand ();

   bool calcVisibility (ObjViewGraph *graph);
};


class OVGAddAssocCommand: public OVGNavigableCommand
{
private:
   char *id2;

protected:
   void doExec ();

public:
   OVGAddAssocCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                       const char *id1, const char *id2,
                       ObjViewFunction *function);
   ~OVGAddAssocCommand ();

   bool calcVisibility (ObjViewGraph *graph);
};


class OVGAddAttrCommand: public OVGNavigableCommand
{
private:
   char *name, *value;
   dw::Label *smallLabel;
   OVGAttribute *attribute;
   int childNo;

protected:
   void doExec ();
   void doShow ();
   void doHide ();

public:
   OVGAddAttrCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                      const char *id, const char *name, const char *value,
                      ObjViewFunction *function);
   ~OVGAddAttrCommand ();

   bool calcVisibility (ObjViewGraph *graph);
};


class OVGDeleteCommand: public OVGNavigableCommand
{
protected:
   void doExec ();

public:
   OVGDeleteCommand (const char *fileName, int lineNo, ObjViewGraph *graph,
                     const char *id, ObjViewFunction *function);
   ~OVGDeleteCommand ();

   bool calcVisibility (ObjViewGraph *graph);
};

} // namespace objects

} // namespace rtfl

#endif // __OBJECTS_OBJVIEW_COMMANDS_HH__
