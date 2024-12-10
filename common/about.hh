#ifndef __COMMON_ABOUT_HH__
#define __COMMON_ABOUT_HH__

#include <FL/Fl_Window.H>

namespace rtfl {

namespace common {

class AboutWindow: public Fl_Window
{
private:
   char *title, *text;

   static void close (Fl_Widget *widget, void *data);

   enum { WIDTH = 450, BUTTON_WIDTH = 80, BUTTON_HEIGHT = 25, SPACE = 10 };

public:
   enum { HEIGHT_SIMPLE = 300, HEIGHT_EXCEPTION = 480 };

   AboutWindow (const char *prgName, const char *licenceException, int height);
   ~AboutWindow ();
};

} // namespace common

} // namespace rtfl

#endif // __COMMON_ABOUT_HH__
