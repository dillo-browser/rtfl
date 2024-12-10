#ifndef __DWR_BOX_HH__
#define __DWR_BOX_HH__

#include "hideable.hh"

namespace rtfl {

namespace dw {

class Box: public Hideable
{
private:
   class BoxIterator: public ::dw::core::Iterator
   {
   private:
      int index;

      BoxIterator (Box *box, ::dw::core::Content::Type mask, int index);

   public:
      BoxIterator (Box *box, ::dw::core::Content::Type mask, bool atEnd);

      lout::object::Object *clone ();
      int compareTo (lout::object::Comparable *other);

      bool next ();
      bool prev ();
      void highlight (int start, int end, ::dw::core::HighlightLayer layer);
      void unhighlight (int direction, ::dw::core::HighlightLayer layer);
      void getAllocation (int start, int end,
                          ::dw::core::Allocation *allocation);
   };

   bool inDestructor;
   static const char countZeroTable[256];

   /**
    * \brief Calculate the number of bits 0 on the left.
    *
    * This is similar to calculating the logarithm base 2 (which finds
    * the right-most bit 1), and a lookup as described at
    * <http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup>
    * is used.
    */
   static inline int getZeroBits (int i)
   {
      // TODO This should not work for 64 bit integers.
      unsigned int ih, ilh, ihh;
      if ((ih = i >> 16))
         return ((ihh = ih >> 8)) ?
            countZeroTable[ihh] : 8 + countZeroTable[ih];
      else
         return ((ilh = i >> 8)) ?
            16 + countZeroTable[ilh] : 24 + countZeroTable[i];
   }

protected:
   lout::container::typed::Vector<Widget> *children;
   bool stretchChildren;

   void sizeRequestImplImpl (::dw::core::Requisition *requisition);
   void actualSizeRequestImplImpl (::dw::core::Requisition *requisition,
                                   int data1);
   void getExtremesImplImpl (::dw::core::Extremes *extremes);

   void drawImpl (::dw::core::View *view, ::dw::core::Rectangle *area);

   /**
    * \brief Return a * b / c, but avoid overflow for large a and b.
    */
   static inline unsigned int safeATimesBDividedByC (unsigned int a,
                                                     unsigned int b,
                                                     unsigned int c)
   {
      // TODO This should not work for 64 bit integers.
      int z = getZeroBits (a) + getZeroBits (b);
      if (z >= 32)
         return a * b / c;
      else {
         //int n = 32 - z, n1 = n / 2, n2 = (n + 1) / 2;
         int n = 32 - z, n1 = 0, n2 = n;
         return (a >> n1) * (b >> n2) / (c >> n);
      }
   }

   virtual void accumulateSize (int index, int size,
                                ::dw::core::Requisition *totalReq,
                                ::dw::core::Requisition *childReq,
                                int data1) = 0;
   virtual void accumulateExtremes (int index, int size,
                                    ::dw::core::Extremes *totalExtr,
                                    ::dw::core::Extremes *childExtr) = 0;

public:
   static int CLASS_ID;

   Box (bool stretchChildren);
   ~Box ();

   ::dw::core::Iterator *iterator (::dw::core::Content::Type mask, bool atEnd);
   void removeChild (Widget *child);

   void addChild (::dw::core::Widget *widget, int newPos = -1);
};

} // namespace rtfl

} // namespace dw

#endif // __DWR_BOX_HH__
