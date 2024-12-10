#ifndef __OBJECTS_OBJVIEW_STRACKTRACE_HH__
#define __OBJECTS_OBJVIEW_STRACKTRACE_HH__

#include <FL/Fl_Window.H>
#include <FL/Fl_Hold_Browser.H>

#include "lout/object.hh"

namespace rtfl {

namespace objects {

class ObjViewFunction
{
public:
   virtual char *createName () = 0;
   virtual void freeName (char *name) = 0;
   virtual int getColor () = 0; // as 0xRRGGBB
   virtual ObjViewFunction *getParent () = 0;
   virtual bool isSelectable () = 0;
   virtual void select () = 0;
};

class ObjViewStacktraceWindow;

// Somewhat simpler than using signals
class ObjViewListener
{
public:
   virtual ~ObjViewListener () { }
   virtual void close (ObjViewStacktraceWindow *window) = 0;
};

class ObjViewStacktraceWindow: public Fl_Window, public lout::object::Object
{
private:
   enum { WIDTH = 450, HEIGHT = 300, BUTTON_WIDTH = 80, BUTTON_HEIGHT = 25,
          SPACE = 10 };

   Fl_Hold_Browser *browser;
   ObjViewFunction *topFunction;
   ObjViewListener *listener;

   static void windowCallback (Fl_Widget *widget, void *data);
   static void jumpTo (Fl_Widget *widget, void *data);
   static void close (Fl_Widget *widget, void *data);

public:
   ObjViewStacktraceWindow (ObjViewFunction *topFunction,
                            ObjViewListener *listener);
   ~ObjViewStacktraceWindow ();

   void update ();
};

} // namespace objects

} // namespace rtfl

#endif // __OBJECTS_OBJVIEW_STRACKTRACE_HH__
