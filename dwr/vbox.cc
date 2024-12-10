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

#include "vbox.hh"
#include "lout/misc.hh"

using namespace dw::core;
using namespace lout::misc;

namespace rtfl {

namespace dw {

int VBox::CLASS_ID = -1;

VBox::VBox (bool stretchChildren):  Box (stretchChildren)
{
   DBG_OBJ_CREATE ("rtfl::dw::VBox");
   registerName ("rtfl::dw::VBox", &CLASS_ID);
}

VBox::~VBox ()
{
   DBG_OBJ_DELETE ();
}

int VBox::findFirstVisibleChild ()
{
   for (int i = 0; i < children->size (); i++) {
      Requisition childReq;
      children->get(i)->sizeRequest (&childReq);
      if (childReq.ascent > 0 || childReq.descent > 0)
         return i;
   }

   return 0;
}

void VBox::sizeRequestImplImpl (Requisition *requisition)
{
   actualSizeRequestImplImpl (requisition, findFirstVisibleChild ());
}

void VBox::sizeAllocateImpl (Allocation *allocation)
{
   // TODO "stretchChildren" is not regarded.
   
   Allocation allocWOBorders;
   allocWOBorders.x = allocation->x + getStyle()->boxOffsetX ();
   allocWOBorders.y = allocation->y + getStyle()->boxOffsetY ();
   allocWOBorders.width = allocation->width - getStyle()->boxDiffWidth ();
   allocWOBorders.ascent = allocation->ascent - getStyle()->boxOffsetY ();
   allocWOBorders.descent = allocation->descent - getStyle()->boxRestHeight ();

   // Unlike HBox, the ascent of the first visible widget (set as
   // allocation ascent) is treated differently; the distribution is
   // only done for the rest. Also, the difference is distributed to
   // ascent (except first widget) and descent independently.

   int sumReqHeight = 0, firstVisibleChild = findFirstVisibleChild ();
   for (int i = firstVisibleChild; i < children->size (); i++) {
      Requisition childReq;
      children->get(i)->sizeRequest (&childReq);
      if (i > firstVisibleChild)
         sumReqHeight += childReq.ascent;
      sumReqHeight += childReq.descent;
   }

   //bool stretch = stretchChildren && sumReqHeight < allocWOBorders.descent;
   
   // Cf. doc/rounding-errors.doc, with: x[i] = child requisition,
   // y[i] = child allocation, a = total allocation, b = sumReqHeight.
   int cumChildReqHeight = 0, cumChildAllocHeight = 0;
   for (int i = 0; i < children->size (); i++) {
      Widget *child = children->get (i);

      Requisition childReq;
      child->sizeRequest (&childReq);

      Allocation childAlloc;
      childAlloc.x = allocWOBorders.x;
      childAlloc.width = allocWOBorders.width;

      if (i == firstVisibleChild) {
         childAlloc.y = allocWOBorders.y;
         childAlloc.ascent = allocWOBorders.ascent;
      } else {
         childAlloc.y =
            allocWOBorders.y + allocWOBorders.ascent + cumChildAllocHeight;
         cumChildReqHeight += childReq.ascent;
         childAlloc.ascent = sumReqHeight > 0 ?
            safeATimesBDividedByC (cumChildReqHeight, allocWOBorders.descent,
                                   sumReqHeight) - cumChildAllocHeight :
            0;
         cumChildAllocHeight += childAlloc.ascent;
      }

      cumChildReqHeight += childReq.descent;
      childAlloc.descent = sumReqHeight > 0 ?
         safeATimesBDividedByC (cumChildReqHeight, allocWOBorders.descent,
                                sumReqHeight) - cumChildAllocHeight :
         0;
      cumChildAllocHeight += childAlloc.descent;
     
      child->sizeAllocate (&childAlloc);
      
      //printf ("vbox: %dth child at (%d, %d), %d, (%d x %d)\n",
      //        i, childAlloc.x, childAlloc.y, childAlloc.width,
      //        childAlloc.ascent, childAlloc.descent);
      
   } 
}

void VBox::accumulateSize (int index, int size, Requisition *totalReq,
                           Requisition *childReq, int data1)
{
   int firstVisibleChild = data1;

   totalReq->width = max (totalReq->width, childReq->width);
   if (index == firstVisibleChild) {
      // Resulting baseline is the baseline of the first child.
      totalReq->ascent = childReq->ascent;
      totalReq->descent = childReq->descent;
   } else 
      totalReq->descent += (childReq->ascent + childReq->descent);
}


void VBox::accumulateExtremes (int index, int size, Extremes *totalExtr,
                               Extremes *childExtr)
{
   totalExtr->minWidth = max (totalExtr->minWidth, childExtr->minWidth);
   totalExtr->maxWidth = max (totalExtr->maxWidth, childExtr->maxWidth);
}

} // namespace rtfl

} // namespace dw
