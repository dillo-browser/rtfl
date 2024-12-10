#ifndef __DWR_HBOX_HH__
#define __DWR_HBOX_HH__

#include "box.hh"

namespace rtfl {

namespace dw {

class HBox: public Box
{
protected:
   void sizeAllocateImpl (::dw::core::Allocation *allocation);
   void accumulateSize (int index, int size, ::dw::core::Requisition *totalReq,
                        ::dw::core::Requisition *childReq, int data1);
   void accumulateExtremes (int index, int size,
                            ::dw::core::Extremes *totalExtr,
                            ::dw::core::Extremes *childExtr);

public:
   static int CLASS_ID;

   HBox (bool stretchChildren);
   ~HBox ();
};

} // namespace rtfl

} // namespace dw

#endif // __DWR_HBOX_HH__
