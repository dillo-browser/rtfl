/*
 * RTFL
 *
 * Copyright 2013-2015 Sebastian Geerken <sgeerken@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version; with the following exception:
 *
 * The copyright holders of RTFL give you permission to link this file
 * statically or dynamically against all versions of the graphviz
 * library, which are published by AT&T Corp. under one of the following
 * licenses:
 *
 * - Common Public License version 1.0 as published by International
 *   Business Machines Corporation (IBM), or
 * - Eclipse Public License version 1.0 as published by the Eclipse
 *   Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graph.hh"
#include "tools.hh"

using namespace dw::core;
using namespace dw::core::style;
using namespace lout::container::typed;
using namespace lout::misc;

namespace rtfl {

namespace dw {

int Graph::CLASS_ID = -1;

// ----------------------------------------------------------------------

// TODO: GraphIterator ignores the text nodes (for references). Are
// they needed, anyway?
Graph::GraphIterator::GraphIterator (Graph *graph, Content::Type mask,
                                     bool atEnd) : Iterator (graph, mask, atEnd)
{
   index = atEnd ? graph->allWidgets->size() : -1;
   content.type = atEnd ? Content::END : Content::START;
}

Graph::GraphIterator::GraphIterator (Graph *graph, Content::Type mask,
                                     int index) : Iterator (graph, mask, false)
{
   this->index = index;

   if (index < 0)
      content.type = Content::START;
   else if (index >= graph->allWidgets->size ())
      content.type = Content::END;
   else {
      content.type = Content::WIDGET_IN_FLOW;
      content.widget = graph->allWidgets->get (index);
   }
}

lout::object::Object *Graph::GraphIterator::clone ()
{
   return new GraphIterator ((Graph*)getWidget(), getMask(), index);
}

int Graph::GraphIterator::compareTo (lout::object::Comparable *other)
{
   return index - ((GraphIterator*)other)->index;
}

bool Graph::GraphIterator::next ()
{
   Graph *graph = (Graph*)getWidget();

   if (content.type == Content::END)
      return false;

   // graphs only contain widgets:
   if ((getMask() & Content::WIDGET_IN_FLOW) == 0) {
      content.type = Content::END;
      return false;
   }

   index++;
   if (index >= graph->allWidgets->size ()) {
      content.type = Content::END;
      return false;
   } else {
      content.type = Content::WIDGET_IN_FLOW;
      content.widget = graph->allWidgets->get (index);
      return true;
   }
}

bool Graph::GraphIterator::prev ()
{
   Graph *graph = (Graph*)getWidget();

   if (content.type == Content::START)
      return false;

   // graphs only contain widgets:
   if ((getMask() & Content::WIDGET_IN_FLOW) == 0) {
      content.type = Content::START;
      return false;
   }

   index--;
   if (index < 0) {
      content.type = Content::START;
      return false;
   } else {
      content.type = Content::WIDGET_IN_FLOW;
      content.widget = graph->allWidgets->get (index);
      return true;
   }
}

void Graph::GraphIterator::highlight (int start, int end, HighlightLayer layer)
{
   /** todo Needs this an implementation? */
}

void Graph::GraphIterator::unhighlight (int direction, HighlightLayer layer)
{
   /** todo Needs this an implementation? */
}

void Graph::GraphIterator::getAllocation (int start, int end,
                                          Allocation *allocation)
{
   /** \bug Not implemented. */
}

// ----------------------------------------------------------------------

Graph::Node::Node (Widget *widget, Node *parent)
{
   type = WIDGET;
   this->widget = widget;
   this->parent = parent;
   children = new List<Node> (true);
}

Graph::Node::Node (const char *text, Node *parent)
{
   type = REF;
   this->text = strdup (text);
   this->parent = parent;
   children = new List<Node> (true);
}

Graph::Node::~Node ()
{
   if (type == WIDGET) {
      if (widget) // see Graph::removeChild
         delete widget;
   } else
      free (text);
   delete children;
}

// ----------------------------------------------------------------------

Graph::Graph ()
{
   DBG_OBJ_CREATE ("rtfl::dw::Graph");
   registerName ("rtfl::dw::Graph", &CLASS_ID);

   allWidgets = new Vector<Widget> (4, false);
   topLevelNodes = new List<Node> (true);
   refStyle = NULL;
   pressedRefNode = NULL;
   inDestructor = false;
}

Graph::~Graph ()
{
   inDestructor = true;
   delete allWidgets;
   delete topLevelNodes;
   if (refStyle)
      refStyle->unref ();

   DBG_OBJ_DELETE ();
}

void Graph::sizeRequestImpl (Requisition *requisition)
{
   sizeRequest (requisition, topLevelNodes);

   requisition->width += getStyle()->boxDiffWidth ();
   requisition->ascent += getStyle()->boxOffsetY ();
   requisition->descent += getStyle()->boxRestHeight ();
}

void Graph::sizeRequest (Requisition *requisition, List<Node> *list)
{
   // For simlicity, descent is set to 0. (Anyway never needed within
   // RTFL.)
   requisition->width = requisition->descent = 0;
   requisition->ascent = max (VDIFF * (list->size () - 1), 0);
   
   for (lout::container::typed::Iterator<Node> it = list->iterator ();
        it.hasNext (); ) {
      Node *node = it.getNext ();
      Requisition nodeReq;
      sizeRequest (&nodeReq, node);
      
      requisition->width = max (requisition->width, nodeReq.width);
      requisition->ascent += (nodeReq.ascent + nodeReq.descent);
   }
}

void Graph::sizeRequest (Requisition *requisition, Node *node)
{
   if (node->type == Node::WIDGET)
      node->widget->sizeRequest (&node->rootReq);
   else {
      node->rootReq.width =
         layout->textWidth (refStyle->font, node->text, strlen (node->text)) +
         refStyle->boxDiffWidth ();
      node->rootReq.ascent = refStyle->font->ascent + refStyle->boxOffsetY ();
      node->rootReq.descent =
         refStyle->font->descent + refStyle->boxRestHeight ();
   }

   if (node->children->isEmpty ())
      *requisition = node->rootReq;
   else {
      sizeRequest (&node->childrenReq, node->children);
      
      requisition->ascent =
         max (node->rootReq.ascent + node->rootReq.descent,
              node->childrenReq.ascent + node->childrenReq.descent);
      requisition->descent = 0;
      requisition->width =
         node->rootReq.width + HDIFF + node->childrenReq.width;
   }
}

void Graph::getExtremesImpl (Extremes *extremes)
{
   getExtremes (extremes, topLevelNodes);

   extremes->minWidth += getStyle()->boxDiffWidth ();
   extremes->maxWidth += getStyle()->boxDiffWidth ();
}

void Graph::getExtremes (Extremes *extremes, List<Node> *list)
{
   extremes->minWidth = extremes->maxWidth = 0;

   for (lout::container::typed::Iterator<Node> it = list->iterator ();
        it.hasNext (); ) {
      Node *node = it.getNext ();
      Extremes nodeExtr;
      getExtremes (&nodeExtr, node);
      
      extremes->minWidth = max (extremes->minWidth, nodeExtr.minWidth);
      extremes->maxWidth = max (extremes->maxWidth, nodeExtr.maxWidth);
   }
}

void Graph::getExtremes (Extremes *extremes, Node *node)
{
   Extremes rootExtr;
   if (node->type == Node::WIDGET)
      node->widget->getExtremes (&rootExtr);
   else
      rootExtr.minWidth = rootExtr.maxWidth =
         layout->textWidth (refStyle->font, node->text, strlen (node->text)) +
         refStyle->boxDiffWidth ();

   if (node->children->isEmpty ())
      *extremes = rootExtr;
   else {
      Extremes childrenExtr;
      getExtremes (&childrenExtr, node->children);
      
      extremes->minWidth = rootExtr.minWidth + HDIFF + childrenExtr.minWidth;
      extremes->maxWidth = rootExtr.maxWidth + HDIFF + childrenExtr.maxWidth;
   }
}

void Graph::sizeAllocateImpl (Allocation *allocation)
{
   // Assumes that sizeRequest has been called before. Only position,
   // not allocation size, is considered.
   //printf ("sizeAllocate (%d, %d, ...)\n", allocation->x, allocation->y);
   sizeAllocate (allocation->x + getStyle()->boxOffsetX (),
                 allocation->y + getStyle()->boxOffsetY (), topLevelNodes);
}

void Graph::sizeAllocate (int x, int y, List<Node> *list)
{
   //printf ("    List: sizeAllocate (%d, %d, ...)\n", x, y);

   for (lout::container::typed::Iterator<Node> it = list->iterator ();
        it.hasNext (); ) {
      Node *node = it.getNext ();
      sizeAllocate (x, y, node);
      int h = node->children->isEmpty () ?
         node->rootReq.ascent + node->rootReq.descent :
         max (node->rootReq.ascent + node->rootReq.descent,
              node->childrenReq.ascent + node->childrenReq.descent);
      y += (VDIFF + h);
   }
}

void Graph::sizeAllocate (int x, int y, Node *node)
{
   // Notice that node->childrenReq is undefined when there are no
   // children.
   
   node->rootX = x;
   node->rootY = node->children->isEmpty () ? y :
      y + max ((node->childrenReq.ascent + node->childrenReq.descent
                - (node->rootReq.ascent + node->rootReq.descent)) / 2, 0);


   if (node->type == Node::WIDGET) {
      Allocation rootAlloc;
      rootAlloc.x = node->rootX;
      rootAlloc.y = node->rootY;
      rootAlloc.width = node->rootReq.width;
      rootAlloc.ascent = node->rootReq.ascent;
      rootAlloc.descent = node->rootReq.descent;
      node->widget->sizeAllocate (&rootAlloc);
   }

   if (!node->children->isEmpty ())
      sizeAllocate (x + HDIFF + node->rootReq.width,
                    y + max ((node->rootReq.ascent + node->rootReq.descent
                              - (node->childrenReq.ascent
                                 + node->childrenReq.descent)) / 2, 0),
                    node->children);
}

void Graph::draw (View *view, Rectangle *area)
{
   DBG_OBJ_ENTER ("draw", 0, "draw", "%d, %d, %d * %d",
                  area->x, area->y, area->width, area->height);

   drawWidgetBox (view, area, false);
   draw (view, area, topLevelNodes);

   DBG_OBJ_LEAVE ();
}

void Graph::draw (View *view, Rectangle *area, List<Node> *list)
{
   for (lout::container::typed::Iterator<Node> it = list->iterator ();
        it.hasNext (); ) {
      Node *node = it.getNext ();
      
      if (node->type == Node::WIDGET) {
         Rectangle childArea;
         if (node->widget->intersects (area, &childArea))
            node->widget->draw (view, &childArea);
      } else {
         drawBox (view, refStyle, area, node->rootX, node->rootY,
                  node->rootReq.width,
                  node->rootReq.ascent + node->rootReq.descent, false);
         view-> drawText (refStyle->font, refStyle->color,
                          Color::SHADING_NORMAL,
                          node->rootX + refStyle->boxOffsetX (),
                          node->rootY + node->rootReq.ascent,
                          node->text, strlen (node->text));
      }

      drawArrows (view, area, node);

      draw (view, area, node->children);
   }
}

void Graph::drawArrows (View *view, Rectangle *area, Node *node)
{
   // TODO Regarding "area" could make this faster, especially for
   // large graphs.

   int x1 = node->rootX + node->rootReq.width;
   int y1 = node->rootY + (node->rootReq.ascent + node->rootReq.descent) / 2;

   for (lout::container::typed::Iterator<Node> it = node->children->iterator ();
        it.hasNext (); ) {
      Node *child = it.getNext ();

      int x2 = child->rootX;
      int y2 =
         child->rootY + (child->rootReq.ascent + child->rootReq.descent) / 2;

      view->drawLine (getStyle()->color, Color::SHADING_NORMAL, x1, y1, x2, y2);
      tools::drawArrowHead (view, getStyle(), x1, y1, x2, y2, AHEADLEN);
   }
}

::dw::core::Iterator *Graph::iterator (Content::Type mask, bool atEnd)
{
   return new GraphIterator (this, mask, atEnd);
}

void Graph::removeChild (Widget *child)
{
   if (!inDestructor) {
      Node *node = searchWidget (child);
      assert (node != NULL);
      assert (node->type == Node::WIDGET);

      // Otherwise, Node::~Node would delete the widget again:
      Widget *widget = node->widget;
      node->widget = NULL;

      // TODO No full implementation, only when all edges have been removed.
      assert (node->parent == NULL);
      assert (node->children->isEmpty ());
         
      bool removed = topLevelNodes->removeRef (node);
      assert (removed);

      int pos = -1;
      for (int i = 0; pos == -1 && i < allWidgets->size (); i++) {
         if (allWidgets->get (i) == widget)
            pos = i;
      }

      assert (pos != -1);
      allWidgets->remove (pos);
   }
}

void Graph::setStyle (Style *style)
{
   Widget::setStyle (style);
   if (refStyle == NULL && style != NULL) {
      refStyle = style;
      refStyle->ref ();
   }
}

Graph::Node *Graph::searchRefNode (MousePositionEvent *event, List<Node> *list)
{
   for (lout::container::typed::Iterator<Node> it = list->iterator ();
        it.hasNext (); ) {
      Node *node = it.getNext ();
      if (event->xCanvas >= node->rootX &&
          event->xCanvas < node->rootX + node->rootReq.width &&
          event->yCanvas >= node->rootY &&
          event->yCanvas < node->rootY + node->rootReq.ascent
          + node->rootReq.descent)
         return node;

      Node *node2 = searchRefNode (event, node->children);
      if (node2)
         return node2;
   }
   
   return NULL;
}

bool Graph::buttonPressImpl (EventButton *event)
{
   if (event->button == 1) {
      pressedRefNode = searchRefNode (event);
      return pressedRefNode != NULL;
   } else
      return false;
}

bool Graph::buttonReleaseImpl (EventButton *event)
{
   if (event->button == 1 && pressedRefNode != NULL) {
      Node *newRefNode = searchRefNode (event);
      if (newRefNode == pressedRefNode) {
         Node *ref = pressedRefNode->reference;
         scrollTo (HPOS_CENTER, VPOS_CENTER, ref->rootX + getAllocation()->x,
                   ref->rootY + getAllocation()->y, ref->rootReq.width,
                   ref->rootReq.ascent + ref->rootReq.descent);
         pressedRefNode = NULL;
      }
      return true;
   } else
      return false;
}

bool Graph::motionNotifyImpl (::dw::core::EventMotion *event)
{
   if ((event->state & BUTTON1_MASK) && pressedRefNode) {
      Node *newRefNode = searchRefNode (event);
      if (newRefNode != pressedRefNode)
         pressedRefNode = NULL;
   }
   return false;
}

void Graph::leaveNotifyImpl (EventCrossing *event)
{
   pressedRefNode = NULL;
}

bool Graph::isAncestorOf (Node *n1, Node *n2)
{
   for (Node *node = n2; node; node = node->parent)
      if (node == n1)
         return true;
   
   return false;         
}

void Graph::addReference (Node *from, Node *to)
{
   assert (from->type == Node::WIDGET);
   assert (to->type == Node::WIDGET);
   //printf ("edge from %s %p to %s %p implemented as reference.\n",
   //        from->widget->getClassName (), from->widget,
   //        to->widget->getClassName (), to->widget);

   Node *ref1 = new Node ("…", from);
   from->children->append (ref1);
   
   Node *ref2 = new Node ("…", to);
   List<Node> *list = to->parent ? to->parent->children : topLevelNodes;
   list->insertBefore (to, ref2);
   list->detachRef (to);
   ref2->children->append (to);

   ref1->reference = ref2;
   ref2->reference = ref1;
}

void Graph::addNode (Widget *widget)
{
   if (searchWidget (widget))
      fprintf (stderr, "WARNING: widget %p added twice.\n", widget);
   else {
      allWidgets->put (widget);
      topLevelNodes->append (new Node (widget, NULL));
      widget->setParent (this);
      queueResize (0, true);
   }
}

void Graph::addEdge (Widget *from, Widget *to)
{
   Node *nodeFrom, *nodeTo;

   if ((nodeFrom = searchWidget (from)) == NULL) {
      fprintf (stderr, "WARNING: adding edge starting from unknown widget %p\n",
               from);
      return;
   }

   if ((nodeTo = searchWidget (to)) == NULL)  {
      fprintf (stderr, "WARNING: adding edge ending in unknown widget %p\n",
               to);
      return;
   }

   if (nodeTo->parent) {
      if (nodeTo->parent == nodeFrom)
         fprintf (stderr,
                  "WARNING: edge from widget %p to widget %p added twice.\n",
                 from, to);
      else {
         // Second node is already the end of an edge, so no new is
         // added, but a reference.
         addReference (nodeFrom, nodeTo);
      }
   } else {
      // Here, nodeTo->parent is NULL.
      if (isAncestorOf (nodeTo, nodeFrom))
         // Would cause a cycle otherwise.
         addReference (nodeFrom, nodeTo);
      else {
         topLevelNodes->detachRef (nodeTo);
         nodeTo->parent = nodeFrom;
         nodeFrom->children->append (nodeTo);
      }
   }

   queueResize (0, true);
}

void Graph::removeEdge (::dw::core::Widget *from, ::dw::core::Widget *to)
{
   Node *nodeFrom, *nodeTo;

   if ((nodeFrom = searchWidget (from)) == NULL) {
      fprintf (stderr, "WARNING: adding edge starting from unknown widget %p\n",
               from);
      return;
   }

   if ((nodeTo = searchWidget (to)) == NULL)  {
      fprintf (stderr, "WARNING: adding edge ending in unknown widget %p\n",
               to);
      return;
   }

   assertNotReached (); // TODO

   queueResize (0, true);
}

Graph::Node *Graph::searchWidget (Widget *widget, List<Node> *list)
{
   for (lout::container::typed::Iterator<Node> it = list->iterator ();
        it.hasNext (); ) {
      Node *node = it.getNext ();
      if (node->type == Node::WIDGET && node->widget == widget)
         return node;

      Node *node2 = searchWidget (widget, node->children);
      if (node2)
         return node2;
   }
   
   return NULL;
}

void Graph::setRefStyle (::dw::core::style::Style *style)
{
   if (refStyle)
      refStyle->unref ();

   refStyle = style;
   refStyle->ref ();
}

} // namespace rtfl

} // namespace dw
