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

#include <math.h>

#include "box.hh"

using namespace dw::core;
using namespace dw::core::style;
using namespace lout::container::typed;
using namespace lout::misc;

namespace rtfl {

namespace dw {

int Box::CLASS_ID = -1;

#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n

const char Box::countZeroTable[256] =  {
   8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
   LT(3), LT(2), LT(2), LT(1), LT(1), LT(1), LT(1),
   LT(0), LT(0), LT(0), LT(0), LT(0), LT(0), LT(0), LT(0)
};

// ----------------------------------------------------------------------

Box::BoxIterator::BoxIterator (Box *box, Content::Type mask,
                                     bool atEnd) : Iterator (box, mask, atEnd)
{
   index = atEnd ? box->children->size() : -1;
   content.type = atEnd ? Content::END : Content::START;
}

Box::BoxIterator::BoxIterator (Box *box, Content::Type mask,
                                     int index) : Iterator (box, mask, false)
{
   this->index = index;

   if (index < 0)
      content.type = Content::START;
   else if (index >= box->children->size ())
      content.type = Content::END;
   else {
      content.type = Content::WIDGET_IN_FLOW;
      content.widget = box->children->get (index);
   }
}

lout::object::Object *Box::BoxIterator::clone ()
{
   return new BoxIterator ((Box*)getWidget(), getMask(), index);
}

int Box::BoxIterator::compareTo (lout::object::Comparable *other)
{
   return index - ((BoxIterator*)other)->index;
}

bool Box::BoxIterator::next ()
{
   Box *box = (Box*)getWidget();

   if (content.type == Content::END)
      return false;

   // boxs only contain widgets:
   if ((getMask() & Content::WIDGET_IN_FLOW) == 0) {
      content.type = Content::END;
      return false;
   }

   index++;
   if (index >= box->children->size ()) {
      content.type = Content::END;
      return false;
   } else {
      content.type = Content::WIDGET_IN_FLOW;
      content.widget = box->children->get (index);
      return true;
   }
}

bool Box::BoxIterator::prev ()
{
   Box *box = (Box*)getWidget();

   if (content.type == Content::START)
      return false;

   // boxs only contain widgets:
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
      content.widget = box->children->get (index);
      return true;
   }
}

void Box::BoxIterator::highlight (int start, int end, HighlightLayer layer)
{
   /** todo Needs this an implementation? */
}

void Box::BoxIterator::unhighlight (int direction, HighlightLayer layer)
{
   /** todo Needs this an implementation? */
}

void Box::BoxIterator::getAllocation (int start, int end,
                                          Allocation *allocation)
{
   /** \bug Not implemented. */
}

// ----------------------------------------------------------------------

Box::Box (bool stretchChildren)
{
   DBG_OBJ_CREATE ("rtfl::dw::Box");
   registerName ("rtfl::dw::Box", &CLASS_ID);

   this->stretchChildren = stretchChildren;
   children = new Vector<Widget> (4, true);
   inDestructor = false;
}

Box::~Box ()
{
   inDestructor = true;
   delete children;

   DBG_OBJ_DELETE ();
}

void Box::sizeRequestImplImpl (Requisition *requisition)
{
   actualSizeRequestImplImpl (requisition, 0);
}

void Box::actualSizeRequestImplImpl (::dw::core::Requisition *requisition,
                                     int data1)
{
   // "data1" is a value passed from a special implementation of
   // sizeRequestImplImpl to accumulateSize. See VBox for an example
   // on usage. Extend by "data2" ... when needed.

   requisition->width = requisition->ascent = requisition->descent = 0;

   for (int i = 0; i < children->size (); i++) {
      Requisition childReq;
      children->get(i)->sizeRequest (&childReq);
      accumulateSize (i, children->size (), requisition, &childReq, data1);
   }      

   requisition->width += getStyle()->boxDiffWidth ();
   requisition->ascent += getStyle()->boxOffsetY ();
   requisition->descent += getStyle()->boxRestHeight ();
}

void Box::getExtremesImplImpl (Extremes *extremes)
{
   extremes->minWidth = extremes->maxWidth = 0;

   for (int i = 0; i < children->size (); i++) {
      Extremes childExtr;
      children->get(i)->getExtremes (&childExtr);
      accumulateExtremes (i, children->size (), extremes, &childExtr);
   }      

   extremes->minWidth += getStyle()->boxDiffWidth ();
   extremes->maxWidth += getStyle()->boxDiffWidth ();
}

void Box::drawImpl (View *view, Rectangle *area)
{
   DBG_OBJ_ENTER ("draw", 0, "draw", "%d, %d, %d * %d",
                  area->x, area->y, area->width, area->height);

   drawWidgetBox (view, area, false);
   for (int i = 0; i < children->size (); i++) {
      Widget *child = children->get (i);
      Rectangle childArea;
      if (child->intersects (area, &childArea))
         child->draw (view, &childArea);
   }

   DBG_OBJ_LEAVE ();
}

::dw::core::Iterator *Box::iterator (Content::Type mask, bool atEnd)
{
   return new BoxIterator (this, mask, atEnd);
}

void Box::removeChild (Widget *child)
{
   if (!inDestructor) {
      for (int i = 0; i < children->size (); i++) {
         if (children->get (i) == child) {
            children->remove (i);
            return;
         }
      }

      assertNotReached ();
   }
}

void Box::addChild (Widget *widget, int newPos)
{
   if (newPos == -1)
      children->put (widget);
   else
      children->insert (widget, newPos);

   widget->setParent (this);
   queueResize (0, true);
}

} // namespace rtfl

} // namespace dw
