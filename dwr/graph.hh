#ifndef __DWR_GRAPH_HH__
#define __DWR_GRAPH_HH__

#include "dw/core.hh"
#include "lout/misc.hh"

namespace rtfl {

namespace dw {

class Graph: public ::dw::core::Widget
{
private:
   class GraphIterator: public ::dw::core::Iterator
   {
   private:
      int index;

      GraphIterator (Graph *graph, ::dw::core::Content::Type mask, int index);

   public:
      GraphIterator (Graph *graph, ::dw::core::Content::Type mask, bool atEnd);

      lout::object::Object *clone ();
      int compareTo (lout::object::Comparable *other);

      bool next ();
      bool prev ();
      void highlight (int start, int end, ::dw::core::HighlightLayer layer);
      void unhighlight (int direction, ::dw::core::HighlightLayer layer);
      void getAllocation (int start, int end,
                          ::dw::core::Allocation *allocation);
   };

   class Node: public lout::object::Object
   {
   public:
      enum { WIDGET, REF } type;
      union {
         ::dw::core::Widget *widget;
         struct {
            char *text;
            Node *reference;
         };
      };
      Node *parent;
      lout::container::typed::List<Node> *children;

      int rootX, rootY;
      ::dw::core::Requisition rootReq, childrenReq;

      Node (::dw::core::Widget *widget, Node *parent);
      Node (const char *text, Node *parent);
      ~Node ();
   };

   enum { HDIFF = 50, VDIFF = 20, AHEADLEN = 10 };

   lout::container::typed::Vector<Widget> *allWidgets;
   lout::container::typed::List<Node> *topLevelNodes;
   ::dw::core::style::Style *refStyle;
   Node *pressedRefNode;
   bool inDestructor;

   Node *searchWidget (::dw::core::Widget *widget,
                       lout::container::typed::List<Node> *list);
   inline Node *searchWidget (::dw::core::Widget *widget)
   { return searchWidget (widget, topLevelNodes); }

   void sizeRequest (::dw::core::Requisition *requisition,
                     lout::container::typed::List<Node> *list);
   void sizeRequest (::dw::core::Requisition *requisition, Node *node);
   void getExtremes (::dw::core::Extremes *extremes,
                     lout::container::typed::List<Node> *list);
   void getExtremes (::dw::core::Extremes *extremes, Node *node);
   void sizeAllocate (int x, int y, lout::container::typed::List<Node> *list);
   void sizeAllocate (int x, int y, Node *node);

   void draw (::dw::core::View *view, ::dw::core::Rectangle *area,
              lout::container::typed::List<Node> *list);
   void drawArrows (::dw::core::View *view, ::dw::core::Rectangle *area,
                    Node *node);

   bool isAncestorOf (Node *n1, Node *n2);
   void addReference (Node *from, Node *to);

   // TODO Not very efficient. Since there are not many ref nodes,
   // they could be kept in one list.
   Node *searchRefNode (::dw::core::MousePositionEvent *event)
   { return searchRefNode (event, topLevelNodes); }
   Node *searchRefNode (::dw::core::MousePositionEvent *event,
                        lout::container::typed::List<Node> *list);

protected:
   void sizeRequestImpl (::dw::core::Requisition *requisition);
   void getExtremesImpl (::dw::core::Extremes *extremes);
   void sizeAllocateImpl (::dw::core::Allocation *allocation);

   bool buttonPressImpl (::dw::core::EventButton *event);
   bool buttonReleaseImpl (::dw::core::EventButton *event);
   bool motionNotifyImpl (::dw::core::EventMotion *event);
   void leaveNotifyImpl (::dw::core::EventCrossing *event);

public:
   static int CLASS_ID;

   Graph ();
   ~Graph ();

   void draw (::dw::core::View *view, ::dw::core::Rectangle *area);
   ::dw::core::Iterator *iterator (::dw::core::Content::Type mask, bool atEnd);
   void removeChild (Widget *child);
   void setStyle (::dw::core::style::Style *style);

   void addNode (::dw::core::Widget *widget);
   void addEdge (::dw::core::Widget *from, ::dw::core::Widget *to);
   void removeEdge (::dw::core::Widget *from, ::dw::core::Widget *to);

   void setRefStyle (::dw::core::style::Style *style);
};

} // namespace rtfl

} // namespace dw

#endif // __DWR_GRAPH_HH__
