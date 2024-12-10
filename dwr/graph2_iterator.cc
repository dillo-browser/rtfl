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

#include "graph2.hh"

using namespace dw::core;

namespace rtfl {

namespace dw {

Graph2::Graph2Iterator::Graph2Iterator (Graph2 *graph, Content::Type mask,
                                        bool atEnd) :
   Iterator (graph, mask, atEnd)
{
   index = atEnd ? graph->nodes->size() : -1;
   content.type = atEnd ? Content::END : Content::START;
}

Graph2::Graph2Iterator::Graph2Iterator (Graph2 *graph, Content::Type mask,
                                        int index) :
   Iterator (graph, mask, false)
{
   this->index = index;

   if (index < 0)
      content.type = Content::START;
   else if (index >= graph->nodes->size ())
      content.type = Content::END;
   else {
      content.type = Content::WIDGET_IN_FLOW;
      content.widget = graph->nodes->get(index)->widget;
   }
}

lout::object::Object *Graph2::Graph2Iterator::clone ()
{
   return new Graph2Iterator ((Graph2*)getWidget(), getMask(), index);
}

int Graph2::Graph2Iterator::compareTo (lout::object::Comparable *other)
{
   return index - ((Graph2Iterator*)other)->index;
}

bool Graph2::Graph2Iterator::next ()
{
   Graph2 *graph = (Graph2*)getWidget();

   if (content.type == Content::END)
      return false;

   // graphs only contain widgets:
   if ((getMask() & Content::WIDGET_IN_FLOW) == 0) {
      content.type = Content::END;
      return false;
   }

   index++;
   if (index >= graph->nodes->size ()) {
      content.type = Content::END;
      return false;
   } else {
      content.type = Content::WIDGET_IN_FLOW;
      content.widget = graph->nodes->get(index)->widget;
      return true;
   }
}

bool Graph2::Graph2Iterator::prev ()
{
   Graph2 *graph = (Graph2*)getWidget();

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
      content.widget = graph->nodes->get(index)->widget;
      return true;
   }
}

void Graph2::Graph2Iterator::highlight (int start, int end,
                                        HighlightLayer layer)
{
   /** todo Needs this an implementation? */
}

void Graph2::Graph2Iterator::unhighlight (int direction, HighlightLayer layer)
{
   /** todo Needs this an implementation? */
}

void Graph2::Graph2Iterator::getAllocation (int start, int end,
                                            Allocation *allocation)
{
   /** \bug Not implemented. */
}

} // namespace rtfl

} // namespace dw
