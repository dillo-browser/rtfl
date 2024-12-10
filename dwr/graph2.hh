#ifndef __DWR_GRAPH2_HH__
#define __DWR_GRAPH2_HH__

#include <graphviz/gvc.h>

#include "dw/core.hh"

namespace rtfl {

namespace dw {

class Graph2: public ::dw::core::Widget
{
private:
   class Graph2Iterator: public ::dw::core::Iterator
   {
   private:
      int index;

      Graph2Iterator (Graph2 *graph, ::dw::core::Content::Type mask, int index);

   public:
      Graph2Iterator (Graph2 *graph, ::dw::core::Content::Type mask,
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

   class Node: public lout::object::Object
   {
   private:
      Graph2 *graph;
      int index;

   public:
      Node (Graph2 *graph, Widget *widget, int index);
      ~Node ();

      void initAg ();
      void cleanupAg ();

      Widget *widget;
      Agnode_t *node;
   };

   class Edge: public lout::object::Object
   {
   private:
      Graph2 *graph;
      int numPointsAlloc, index;

      void cleanupPoints ();

   public:
      Edge (Graph2 *graph, Node *from, Node *to, int index);
      ~Edge ();

      void initAg ();
      void cleanupAg ();

      void setNumPoints (int numPoints);
      void sortPoints ();

      Node *from, *to;
      Agedge_t *edge;
      int numPoints;
      int *pointX, *pointY;
      char *pointType;
      
      int count;
   };

   enum { AHEADLEN = 10 };

   Agraph_t *graph;
   GVC_t *gvc;
   lout::container::typed::Vector<Node> *nodes;
   lout::container::typed::Vector<Edge> *edges;
   bool inDestructor;

   void initAg ();
   void cleanupAg ();

   int searchNodeIndex (Widget *widget);
   inline Node *searchNode (Widget *widget)
   { return nodes->get (searchNodeIndex (widget)); }

protected:
   void sizeRequestImpl (::dw::core::Requisition *requisition);
   void getExtremesImpl (::dw::core::Extremes *extremes);
   void sizeAllocateImpl (::dw::core::Allocation *allocation);

public:
   static int CLASS_ID;

   Graph2 ();
   ~Graph2 ();

   void draw (::dw::core::View *view, ::dw::core::Rectangle *area);
   ::dw::core::Iterator *iterator (::dw::core::Content::Type mask, bool atEnd);
   void removeChild (Widget *child);

   inline void setRefStyle (::dw::core::style::Style *style)
   { /* No need anymore. */ }

   void addNode (Widget *widget);
   void addEdge (::dw::core::Widget *from, ::dw::core::Widget *to);
};

} // namespace rtfl

} // namespace dw

#endif // __DWR_GRAPH2_HH__
