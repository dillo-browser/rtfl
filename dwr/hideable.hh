#ifndef __DWR_HIDEABLE_HH__
#define __DWR_HIDEABLE_HH__

#include "dw/core.hh"

namespace rtfl {

namespace dw {

class Hideable: public ::dw::core::Widget
{
   bool hidden, drawable;
   
protected:
   void sizeRequestImpl (::dw::core::Requisition *requisition);
   void getExtremesImpl (::dw::core::Extremes *extremes);

   virtual void sizeRequestImplImpl (::dw::core::Requisition *requisition) = 0;
   virtual void getExtremesImplImpl (::dw::core::Extremes *extremes) = 0;
   virtual void drawImpl (::dw::core::View *view, ::dw::core::Rectangle *area)
      =  0;

public:
   static int CLASS_ID;

   Hideable ();
   ~Hideable ();

   void draw (::dw::core::View *view, ::dw::core::Rectangle *area);

   void show ();
   void hide ();
};

} // namespace rtfl

} // namespace dw

#endif // __DWR_HIDEABLE_HH__
