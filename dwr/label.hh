#ifndef __DWR_LABEL_HH__
#define __DWR_LABEL_HH__

#include "dwr/hideable.hh"
#include "lout/misc.hh"

namespace rtfl {

namespace dw {

class Label: public Hideable
{
private:
   class LabelIterator: public ::dw::core::Iterator
   {
   private:
      int index;

      LabelIterator (Label *label, ::dw::core::Content::Type mask, int index);

   public:
      LabelIterator (Label *label, ::dw::core::Content::Type mask, bool atEnd);

      lout::object::Object *clone ();
      int compareTo (lout::object::Comparable *other);

      bool next ();
      bool prev ();
      void highlight (int start, int end, ::dw::core::HighlightLayer layer);
      void unhighlight (int direction, ::dw::core::HighlightLayer layer);
      void getAllocation (int start, int end,
                          ::dw::core::Allocation *allocation);
   };

   enum { ITALIC = 2, BOLD = 1 };

   struct Word
   {
      char *text;
      char styleIndex;
      ::dw::core::Requisition size;
   };

   ::dw::core::style::Style *styles[4];
   lout::misc::SimpleVector<Word> *words;
   ::dw::core::Requisition totalSize;
   bool selected, buttonDown;
   int link;
   ::dw::core::Layout::LinkEmitter linkEmitter;

   void clearWords ();
   void clearStyles ();
   void ensureStyles ();
   
protected:
   void sizeRequestImplImpl (::dw::core::Requisition *requisition);
   void getExtremesImplImpl (::dw::core::Extremes *extremes);
   void drawImpl (::dw::core::View *view, ::dw::core::Rectangle *area);

   bool buttonPressImpl (::dw::core::EventButton *event);
   bool buttonReleaseImpl (::dw::core::EventButton *event);
   void leaveNotifyImpl (::dw::core::EventCrossing *event);

public:
   static int CLASS_ID;

   Label (const char *text, int link = -1);
   ~Label ();

   void setText (const char *text);
   inline void setLink (int link) { this->link = link; }

   ::dw::core::Iterator *iterator (::dw::core::Content::Type mask, bool atEnd);
   void setStyle (::dw::core::style::Style *style);

   void select ();
   void unselect ();

   inline void connectLink (::dw::core::Layout::LinkReceiver *receiver)
   { linkEmitter.connectLink (receiver); }
};

} // namespace rtfl

} // namespace dw

#endif // __DWR_LABEL_HH__
