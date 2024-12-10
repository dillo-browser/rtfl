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

#include "hbox.hh"

using namespace dw::core;
using namespace lout::misc;

namespace rtfl {

namespace dw {

int HBox::CLASS_ID = -1;

HBox::HBox (bool stretchChildren):  Box (stretchChildren)
{
   DBG_OBJ_CREATE ("rtfl::dw::HBox");
   registerName ("rtfl::dw::HBox", &CLASS_ID);
}

HBox::~HBox ()
{
   DBG_OBJ_DELETE ();
}

void HBox::sizeAllocateImpl (Allocation *allocation)
{
   Allocation allocWOBorders;
   allocWOBorders.x = allocation->x + getStyle()->boxOffsetX ();
   allocWOBorders.y = allocation->y + getStyle()->boxOffsetY ();
   allocWOBorders.width = allocation->width - getStyle()->boxDiffWidth ();
   allocWOBorders.ascent = allocation->ascent - getStyle()->boxOffsetY ();
   allocWOBorders.descent = allocation->descent - getStyle()->boxRestHeight ();

   int sumReqWidth = 0;
   for (int i = 0; i < children->size (); i++) {
      Requisition childReq;
      children->get(i)->sizeRequest (&childReq);
      sumReqWidth += childReq.width;
   }

   bool stretch = stretchChildren && sumReqWidth < allocWOBorders.width;

   // Cf. doc/rounding-errors.doc, with: x[i] = child requisition,
   // y[i] = child allocation, a = total allocation, b = sumReqWidth.
   int cumChildReqWidth = 0, cumChildAllocWidth = 0;
   for (int i = 0; i < children->size (); i++) {
      Widget *child = children->get (i);
      
      Requisition childReq;
      child->sizeRequest (&childReq);
         
      Allocation childAlloc;
      childAlloc.x = allocWOBorders.x + cumChildAllocWidth;
      childAlloc.y = allocWOBorders.y;

      if (stretch) {
         cumChildReqWidth += childReq.width;
         childAlloc.width = sumReqWidth > 0 ?
            safeATimesBDividedByC (cumChildReqWidth, allocWOBorders.width,
                                   sumReqWidth) - cumChildAllocWidth :
            0;
         childAlloc.ascent = allocWOBorders.ascent;
         childAlloc.descent = allocWOBorders.descent;
      } else
         childAlloc.width = childReq.width;
      
      childAlloc.ascent = allocWOBorders.ascent;
      childAlloc.descent = allocWOBorders.descent;
      cumChildAllocWidth += childAlloc.width;
         
      child->sizeAllocate (&childAlloc);        
      
      //printf ("hbox: %dth child at (%d, %d), %d, (%d x %d)\n",
      //        i, childAlloc.x, childAlloc.y, childAlloc.width,
      //        childAlloc.ascent, childAlloc.descent);
   } 
}

void HBox::accumulateSize (int index, int size, Requisition *totalReq,
                           Requisition *childReq, int data1)
{
   totalReq->width += childReq->width;
   totalReq->ascent = max (totalReq->ascent, childReq->ascent);
   totalReq->descent = max (totalReq->descent, childReq->descent);
}

void HBox::accumulateExtremes (int index, int size, Extremes *totalExtr,
                               Extremes *childExtr)
{
   totalExtr->minWidth += childExtr->minWidth;
   totalExtr->maxWidth += childExtr->maxWidth;
}

} // namespace rtfl

} // namespace dw
