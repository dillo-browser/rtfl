#ifndef __DWR_VBOX_HH__
#define __DWR_VBOX_HH__

#include "box.hh"

namespace rtfl {

namespace dw {

class VBox: public Box
{
private:
   int findFirstVisibleChild ();

protected:
   void sizeRequestImplImpl (::dw::core::Requisition *requisition);
   void sizeAllocateImpl (::dw::core::Allocation *allocation);
   void accumulateSize (int index, int size, ::dw::core::Requisition *totalReq,
                        ::dw::core::Requisition *childReq, int data1);
   void accumulateExtremes (int index, int size,
                            ::dw::core::Extremes *totalExtr,
                            ::dw::core::Extremes *childExtr);

public:
   static int CLASS_ID;

   VBox (bool stretchChildren);
   ~VBox ();
};

} // namespace rtfl

} // namespace dw

#endif // __DWR_VBOX_HH__
