#ifndef __DWR_TOGGLE_HH__
#define __DWR_TOGGLE_HH__

#include "hideable.hh"

namespace rtfl {

namespace dw {

class Toggle: public Hideable
{
private:
   class ToggleIterator: public ::dw::core::Iterator
   {
   private:
      int index ();

   public:
      ToggleIterator (Toggle *toggle, ::dw::core::Content::Type mask,
                      bool atEnd);

      lout::object::Object *clone ();
      int compareTo (lout::object::Comparable *other);

      bool next ();
      bool prev ();
      void highlight (int start, int end, ::dw::core::HighlightLayer layer);
      void unhighlight (int direction, ::dw::core::HighlightLayer layer);
      void getAllocation (int start, int end,
                          ::dw::core::Allocation *allocation);
   };

   static enum ButtonStyle { PLUS_MINUS, TRIANGLE } buttonStyle;
    
   bool showLarge, buttonDown;
   Widget *small, *large;

   bool insideButton (::dw::core::MousePositionEvent *event);
   inline int calcButtonSize ()
   { // Always return an odd number.
      int s = getStyle()->font->ascent; return (s % 2) ? s : s - 1; }

protected:
   void sizeRequestImplImpl (::dw::core::Requisition *requisition);
   void getExtremesImplImpl (::dw::core::Extremes *extremes);
   void sizeAllocateImpl (::dw::core::Allocation *allocation);

   void drawImpl (::dw::core::View *view, ::dw::core::Rectangle *area);

   bool buttonPressImpl (::dw::core::EventButton *event);
   bool buttonReleaseImpl (::dw::core::EventButton *event);
   bool motionNotifyImpl (::dw::core::EventMotion *event);
   void leaveNotifyImpl (::dw::core::EventCrossing *event);

public:
   static int CLASS_ID;

   Toggle (bool showLarge);
   ~Toggle ();

   ::dw::core::Iterator *iterator (::dw::core::Content::Type mask, bool atEnd);
   void removeChild (Widget *child);

   inline ::dw::core::Widget *getSmall () { return small; }
   inline ::dw::core::Widget *getLarge () { return large; }
   void setSmall (::dw::core::Widget *widget);
   void setLarge (::dw::core::Widget *widget);

   inline bool isLargeShown () { return showLarge; }
   void toggle (bool showLarge);
};

} // namespace rtfl

} // namespace dw

#endif // __DWR_TOGGLE_HH__
