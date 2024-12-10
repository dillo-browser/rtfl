/*
 * RTFL
 *
 * Copyright 2014, 2015 Sebastian Geerken <sgeerken@dillo.org>
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

/* ----------------------------------------------------------------------

   This is an experimenal graph widget which replaces the widget Graph
   currently used by rtfl-objview. It uses Graphviz for layouting; in
   detail, pixels and points (1 point = 1/72 of an inch) are treated
   as identical, for attributes "bb", "pos" etc. ("width" and "height"
   are given in inch, so these values are calculated by division by
   72).

   See <http://www.graphviz.org/doc/info/attrs.html> for informations
   on Graphviz attributes.

   Open issues:

   - The order of the children should be preserved. When B1, B2 and B3
     are associated to A, in this order, this order should be visible,
     even at the expense of other factors.

     (My original idea was to implement this widget from scratch,
     using a force directed approach with 

       (i) invisible edges between the "children" (in the example
           above: B1 -> B2 and B2 -> B3), which cause attraction of
           the respective nodes in a similar way the visible nodes do
           ("springs");

      (ii) a homogeneous force field directed to the right and acting
           on the heads of the *visible* edges, so giving the graph
           layout an orientation from left to right;

     (iii) a second homogeneous force field directed to the bottom,
           which acts only on the heads of the *invisible* edges.

     (i) and (iii) would lead to the desired result.)

     Update: setting "ordering" to "out" is a step into this
     direction. (This seems to enforce the order of the edges, not the
     nodes.)

   - Configurability: the algorithm (currently "dot") and some more
     parameters (like "rankdir=LR") could be made configurable.

   ---------------------------------------------------------------------- */

#include "graph2.hh"
#include "tools.hh"
#include "../common/tools.hh"
#include "../lout/misc.hh"

using namespace dw::core;
using namespace dw::core::style;
using namespace lout::container::typed;
using namespace lout::misc;

namespace rtfl {

namespace dw {

int Graph2::CLASS_ID = -1;

Graph2::Node::Node (Graph2 *graph, Widget *widget, int index)
{
   this->graph = graph;
   this->widget = widget;
   this->index = index;

   initAg ();
}

Graph2::Node::~Node ()
{
   if (widget)
      delete widget;

   cleanupAg ();
}

void Graph2::Node::initAg ()
{
   assert (graph->graph);

   char buf[64];
   snprintf (buf, 64, "n%d", index);
   node = agnode(graph->graph, buf, TRUE);
   agsafeset (node, (char*)"shape", (char*)"rect", (char*)"");
}

void Graph2::Node::cleanupAg ()
{
   if (node) {
      assert (graph->graph);
      agdelete (graph->graph, node);
      node = NULL;
   }
}

Graph2::Edge::Edge (Graph2 *graph, Node *from, Node *to, int index)
{
   this->graph = graph;
   this->from = from;
   this->to = to;
   this->index = index;
   
   initAg ();

   numPoints = numPointsAlloc = 0;
   pointX = NULL;
   pointY = NULL;
   pointType = NULL;

   count = 1;
}

Graph2::Edge::~Edge ()
{
   cleanupAg ();
   cleanupPoints ();
}

void Graph2::Edge::initAg ()
{
   assert (graph->graph);

   char buf[64];
   snprintf (buf, 64, "e%d", index);
   edge = agedge (graph->graph, from->node, to->node, (char*)buf, TRUE);
}

void Graph2::Edge::cleanupAg ()
{
   if (edge) {
      assert (graph->graph);
      agdelete (graph->graph, edge);
      edge = NULL;
   }
}

void Graph2::Edge::cleanupPoints ()
{
   if (numPointsAlloc > 0) {
      delete[] pointX;
      delete[] pointY;
      delete[] pointType;

      pointX = NULL;
      pointY = NULL;
      pointType = NULL;

      numPointsAlloc = 0;
   }
}

void Graph2::Edge::setNumPoints (int numPoints)
{
   if (numPoints > numPointsAlloc) {
      cleanupPoints ();

      numPointsAlloc = numPoints;

      pointX = new int[numPointsAlloc];
      pointY = new int[numPointsAlloc];
      pointType = new char[numPointsAlloc];
   }

   this->numPoints = numPoints;
}

void Graph2::Edge::sortPoints ()
{
   // If a start point exists, it is at the beginning. If an end point
   // exists, it is also at the beginning or follows the start point.
   // Here, it is moved at the end of the list.

   if ((numPoints >= 1 && pointType[0] == 'e') ||
       (numPoints >= 2 && pointType[1] == 'e')) {
      int epos = pointType[0] == 'e' ? 0 : 1;
      int ex = pointX[epos], ey = pointY[epos];

      for (int i = epos; i < numPoints - 1; i++) {
         pointX[i] = pointX[i + 1];
         pointY[i] = pointY[i + 1];
         pointType[i] = pointType[i + 1];
      }

      pointX[numPoints - 1] = ex;
      pointY[numPoints - 1] = ey;
      pointType[numPoints - 1] = 'e';
   }
}

Graph2::Graph2 ()
{
   DBG_OBJ_CREATE ("rtfl::dw:"###);
   registerName ("rtfl::dw::Graph2", &CLASS_ID);

   nodes = new Vector<Node> (4, true);
   edges = new Vector<Edge> (4, true);

   initAg ();
}

Graph2::~Graph2 ()
{
   inDestructor = true;

   cleanupAg ();

   delete nodes;
   delete edges;

   DBG_OBJ_DELETE ();
}

void Graph2::initAg ()
{
   gvc = gvContext ();
   graph = agopen ((char*)"", Agdirected, NULL);
   agsafeset (graph, (char*)"rankdir", (char*)"LR", (char*)"");
   agsafeset (graph, (char*)"ordering", (char*)"out", (char*)"");

   for (int i = 0; i < nodes->size (); i++)
      nodes->get(i)->initAg ();

   for (int i = 0; i < edges->size (); i++)
      edges->get(i)->initAg ();
}

void Graph2::cleanupAg ()
{
   for (int i = 0; i < nodes->size (); i++)
      nodes->get(i)->cleanupAg ();

   for (int i = 0; i < edges->size (); i++)
      edges->get(i)->cleanupAg ();

   if (graph) {
      agclose (graph);  
      graph = NULL;
   }

   if (gvc) {
      gvFreeContext(gvc);
      gvc = NULL;
   }
}

void Graph2::sizeRequestImpl (Requisition *requisition)
{
   for (int i = 0; i < nodes->size (); i++) {
      Node *node = nodes->get (i);

      Requisition childReq;
      node->widget->sizeRequest (&childReq);

      char buf[64];
      snprintf (buf, 64, "%f", (float)childReq.width / 72);
      agsafeset (node->node, (char*)"width", buf, (char*)"");
      snprintf (buf, 64, "%f",
                (float)(childReq.ascent + childReq.descent) / 72);
      agsafeset (node->node, (char*)"height", buf, (char*)"");
   }

   //puts ("---------- before layouting ----------");
   //agwrite (graph, stdout);

   gvLayout (gvc, graph, "dot");
   gvRender(gvc, graph, "dot", NULL);
   gvFreeLayout(gvc, graph);
 
   //puts ("---------- after layouting ----------");
   //agwrite (graph, stdout);

   float x, y, width, height;
   sscanf (agget (graph, (char*)"bb"), "%f,%f,%f,%f", &x, &y, &width, &height);
   
   requisition->width = width + getStyle()->boxDiffWidth ();
   requisition->ascent = height + getStyle()->boxOffsetY ();
   requisition->descent = getStyle()->boxRestHeight ();
}

void Graph2::getExtremesImpl (Extremes *extremes)
{
   assertNotReached ();
}

void Graph2::sizeAllocateImpl (Allocation *allocation)
{
   //agwrite (graph, stdout);

   Requisition req;
   sizeRequest (&req);
   
   for (int i = 0; i < nodes->size (); i++) {
      Node *node = nodes->get (i);

      Requisition childReq;
      node->widget->sizeRequest (&childReq);

      float x, y;
      sscanf (agget (node->node, (char*)"pos"), "%f,%f", &x, &y);

      Allocation childAlloc;
      childAlloc.x = getStyle()->boxOffsetX () + x - childReq.width / 2;
      childAlloc.y = getStyle()->boxOffsetY ()
         + (req.ascent + req.descent - getStyle()->boxDiffHeight ())
         - y - (childReq.ascent + childReq.descent) / 2;
      childAlloc.width = childReq.width;
      childAlloc.ascent = childReq.ascent;
      childAlloc.descent = childReq.descent;
      node->widget->sizeAllocate (&childAlloc);
   }

   for (int i = 0; i < edges->size (); i++) {
      Edge *edge = edges->get (i);

      char *pos = agget (edge->edge, (char*)"pos");
      int lenPos = strlen (pos);
      char *posBuf = new char[lenPos + 1];
      strncpy (posBuf, pos, lenPos + 1);

      int numSpaces = 0;
      for (char *s = posBuf; *s; s++)
         if (*s == ' ')
            numSpaces++;

      edge->setNumPoints (numSpaces + 1);

      int noPoint = 0;
      bool atEnd = false;
      for (char *start = posBuf; !atEnd; ) {
         char *end = start;
         while (*end != ' ' && *end != 0)
            end++;
         atEnd = (*end == 0);

         *end = 0;

         char pointType, *posStart;
         if ((start[0] == 's' || start[0] == 'e') && start[1] == ',') {
            pointType = start[0];
            posStart = start + 2;
         } else {
            pointType = 0;
            posStart = start;
         }

         float x, y;
         sscanf (posStart, "%f,%f", &x, &y);

         edge->pointX[noPoint] = getStyle()->boxOffsetX () + x;
         edge->pointY[noPoint] = getStyle()->boxOffsetY ()
            + (req.ascent + req.descent - getStyle()->boxDiffHeight ()) - y;
         edge->pointType[noPoint] = pointType;

         noPoint++;
         start = end + 1;
      }

      edge->sortPoints ();

      delete[] posBuf;
   }      
}

void Graph2::draw (View *view, Rectangle *area)
{
   DBG_OBJ_ENTER ("draw", 0, "draw", "%d, %d, %d * %d",
                  area->x, area->y, area->width, area->height);

   drawWidgetBox (view, area, false);

   for (int i = 0; i < nodes->size (); i++) {
      Widget *widget = nodes->get(i)->widget;
      Rectangle childArea;
      if (widget->intersects (area, &childArea))
         widget->draw (view, &childArea);
   }

   for (int i = 0; i < edges->size (); i++) {
      Edge *edge = edges->get (i);

      tools::drawBSpline (view, getStyle(), 4, edge->numPoints, edge->pointX,
                          edge->pointY);

      for (int j = 0; j < edge->numPoints - 1; j++) {
         // TODO: arrow heads should only be drawn at the first and the last
         // point. In this case, the direction is correct; in other cases, the
         // the points are not even part of the curve.

         if (edge->pointType[j] == 'e')
            tools::drawArrowHead (view, getStyle (),
                                  edge->pointX[j + 1], edge->pointY[j + 1],
                                  edge->pointX[j], edge->pointY[j],
                                  AHEADLEN);
            
         if (edge->pointType[j + 1] == 'e')
            tools::drawArrowHead (view, getStyle (),
                                  edge->pointX[j], edge->pointY[j],
                                  edge->pointX[j + 1], edge->pointY[j + 1],
                                  AHEADLEN);
      }
   }

   DBG_OBJ_LEAVE ();
}

::dw::core::Iterator *Graph2::iterator (Content::Type mask, bool atEnd)
{
   return new Graph2Iterator (this, mask, atEnd);
}

void Graph2::removeChild (Widget *child)
{
   if (!inDestructor) {
      int nodeIndex = searchNodeIndex (child);
      Node *node = nodes->get (nodeIndex);

      // Otherwise, Node::~Node would delete the widget again:
      node->widget = NULL;

      for (int i = 0; i < edges->size (); i++) {
         Edge *edge = edges->get (i);
         if (edge->from == node || edge->to == node)
            edges->remove (i);
      }

      nodes->remove (nodeIndex);

      queueResize (0, true);
   }
}

void Graph2::addNode (Widget *widget)
{
   Node *node = new Node (this, widget, nodes->size ());
   nodes->put (node);

   widget->setParent (this);

   queueResize (0, true);
}

void Graph2::addEdge (Widget *from, Widget *to)
{
   for (int i = 0; i < edges->size (); i++) {
      Edge *edge = edges->get (i);
      if (edge->from->widget == from && edge->to->widget == to) {
         edge->count++;
         printf ("WARNING: Edge already added the %d%s time.\n", edge->count,
                 rtfl::tools::numSuffix (edge->count));
         return;
      }
   }

   Edge *edge =
      new Edge (this, searchNode (from), searchNode (to), edges->size ());
   edges->put (edge);

   queueResize (0, true);
}

int Graph2::searchNodeIndex (Widget *widget)
{
   for (int i = 0; i < nodes->size (); i++) {
      Node *node = nodes->get (i);
      if (node->widget == widget)
         return i;
   }

   assertNotReached ();
   return 0;
}

} // namespace rtfl

} // namespace dw
