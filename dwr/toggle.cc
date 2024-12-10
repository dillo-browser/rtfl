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

#include "toggle.hh"

using namespace dw::core;
using namespace dw::core::style;
using namespace lout::misc;

namespace rtfl {

namespace dw {

int Toggle::CLASS_ID = -1;

Toggle::ButtonStyle Toggle::buttonStyle = PLUS_MINUS;

// ----------------------------------------------------------------------

Toggle::ToggleIterator::ToggleIterator (Toggle *toggle, Content::Type mask,
                                        bool atEnd) :
   Iterator (toggle, mask, atEnd)
{
   content.type = atEnd ? Content::END : Content::START;
}

lout::object::Object *Toggle::ToggleIterator::clone ()
{
   ToggleIterator *ti =
      new ToggleIterator ((Toggle*)getWidget(), getMask(), false);
   ti->content = content;
   return ti;
}

int Toggle::ToggleIterator::index ()
{
   switch (content.type) {
   case Content::START:
      return 0;
   case Content::WIDGET_IN_FLOW:
      return content.widget == ((Toggle*)getWidget())->small ? 1 : 2;
   case Content::END:
      return 3;
   default:
      assertNotReached ();
      return 0;
   }
}

int Toggle::ToggleIterator::compareTo (lout::object::Comparable *other)
{
   return index () - ((ToggleIterator*)other)->index ();     
}

bool Toggle::ToggleIterator::next ()
{
   Toggle *toggle = (Toggle*)getWidget();

   if (content.type == Content::END)
      return false;

   // toggles only contain widgets:
   if ((getMask() & Content::WIDGET_IN_FLOW) == 0) {
      content.type = Content::END;
      return false;
   }

   if (content.type == Content::START) {
      if (toggle->small != NULL) {
         content.type = Content::WIDGET_IN_FLOW;
         content.widget = toggle->small;
         return true;
      } else if (toggle->large != NULL) {
         content.type = Content::WIDGET_IN_FLOW;
         content.widget = toggle->large;
         return true;
      } else {
         content.type = Content::END;
         return false;
      }
   } else /* if (content.type == Content::WIDGET) */ {
      if (content.widget == toggle->small && toggle->large != NULL) {
         content.widget = toggle->large;
         return true;
      } else {
         content.type = Content::END;
         return false;
      }
   }
}

bool Toggle::ToggleIterator::prev ()
{
   Toggle *toggle = (Toggle*)getWidget();

   if (content.type == Content::START)
      return false;

   // toggles only contain widgets:
   if ((getMask() & Content::WIDGET_IN_FLOW) == 0) {
      content.type = Content::START;
      return false;
   }

   if (content.type == Content::END) {
      if (toggle->large != NULL) {
         content.type = Content::WIDGET_IN_FLOW;
         content.widget = toggle->large;
         return true;
      } else if (toggle->small != NULL) {
         content.type = Content::WIDGET_IN_FLOW;
         content.widget = toggle->small;
         return true;
      } else {
         content.type = Content::START;
         return false;
      }
   } else /* if (content.type == Content::WIDGET) */ {
      if (content.widget == toggle->large && toggle->small != NULL) {
         content.widget = toggle->small;
         return true;
      } else {
         content.type = Content::START;
         return false;
      }
   }
}

void Toggle::ToggleIterator::highlight (int start, int end, HighlightLayer layer)
{
   /** todo Needs this an implementation? */
}

void Toggle::ToggleIterator::unhighlight (int direction, HighlightLayer layer)
{
   /** todo Needs this an implementation? */
}

void Toggle::ToggleIterator::getAllocation (int start, int end,
                                          Allocation *allocation)
{
   /** \bug Not implemented. */
}

// ----------------------------------------------------------------------

Toggle::Toggle (bool showLarge)
{
   DBG_OBJ_CREATE ("rtfl::dw::Toggle");
   registerName ("rtfl::dw::Toggle", &CLASS_ID);

   this->showLarge = showLarge;
   small = large = NULL;
   buttonDown = false;
}

Toggle::~Toggle ()
{
   if (small)
      delete small;
   if (large)
      delete large;

   DBG_OBJ_DELETE ();
}

void Toggle::sizeRequestImplImpl (Requisition *requisition)
{
   Widget *child = showLarge ? large : small;
   Requisition childReq;
   if (child)
      child->sizeRequest (&childReq);
   else
      childReq.width = childReq.ascent = childReq.descent = 0;
   
   int buttonSize = calcButtonSize ();
   requisition->width =
      buttonSize + childReq.width + getStyle()->boxDiffWidth ();
   requisition->ascent =
      max (buttonSize, childReq.ascent) + getStyle()->boxOffsetY ();
   requisition->descent = childReq.descent + getStyle()->boxRestHeight ();
}


void Toggle::getExtremesImplImpl (Extremes *extremes)
{
   Widget *child = showLarge ? large : small;
   Extremes childExtr;
   if (child)
      child->getExtremes (&childExtr);
   else
      childExtr.minWidth = childExtr.maxWidth = 0;

   int buttonSize = calcButtonSize ();
   extremes->minWidth =
      buttonSize + childExtr.minWidth + getStyle()->boxDiffWidth ();
   extremes->maxWidth =
      buttonSize + childExtr.maxWidth + getStyle()->boxDiffWidth ();
}


void Toggle::sizeAllocateImpl (Allocation *allocation)
{
   Widget *visChild = showLarge ? large : small;
   Widget *invisChild = showLarge ? small : large;
   Allocation childAlloc;
   int buttonSize = calcButtonSize ();

   if (visChild) {
      childAlloc.x = allocation->x + buttonSize + getStyle()->boxOffsetX ();
      childAlloc.y = allocation->y + getStyle()->boxOffsetY ();
      childAlloc.width =
         allocation->width - buttonSize - getStyle()->boxDiffWidth ();
      childAlloc.ascent = allocation->ascent - getStyle()->boxOffsetY ();
      childAlloc.descent = allocation->descent - getStyle()->boxRestHeight ();
      visChild->sizeAllocate (&childAlloc);
   }

   //printf ("toggle: visible child at (%d, %d), %d, (%d x %d)\n",
   //        childAlloc.x, childAlloc.y, childAlloc.width,
   //        childAlloc.ascent, childAlloc.descent);
    
   if (invisChild) {
      // Zero size is used to hide a widget.
      childAlloc.width = childAlloc.ascent = childAlloc.descent = 0;
      invisChild->sizeAllocate (&childAlloc);
   }
}

void Toggle::drawImpl (View *view, Rectangle *area)
{
   DBG_OBJ_ENTER ("draw", 0, "draw", "%d, %d, %d * %d",
                  area->x, area->y, area->width, area->height);

   drawWidgetBox (view, area, false);
   Widget *child = showLarge ? large : small;
   Rectangle childArea;
   if (child && child->intersects (area, &childArea))
      child->draw (view, &childArea);

   int buttonSize = calcButtonSize ();
   int x0 = getAllocation()->x + getStyle()->boxOffsetX ();
   int y0 = getAllocation()->y + getAllocation()->ascent - buttonSize;

   switch (buttonStyle) {
   case PLUS_MINUS:
      view->drawLine (getStyle()->color, Color::SHADING_NORMAL,
                      x0 + 1, y0 + buttonSize / 2,
                      x0 + buttonSize - 2, y0 + buttonSize / 2);
      if (!showLarge)
         view->drawLine (getStyle()->color, Color::SHADING_NORMAL,
                         x0 + buttonSize / 2, y0 + 1,
                         x0 + buttonSize / 2, y0 + buttonSize - 2);
      break;

   case TRIANGLE:
      if (showLarge) {
         view->drawLine (getStyle()->color, Color::SHADING_NORMAL,
                         x0 + 1, y0 + 1, x0 + buttonSize - 2, y0 + 1);
         view->drawLine (getStyle()->color, Color::SHADING_NORMAL,
                         x0 + 1, y0 + 1,
                         x0 + buttonSize / 2, y0 + buttonSize - 2);
         view->drawLine (getStyle()->color, Color::SHADING_NORMAL,
                         x0 + buttonSize - 2, y0 + 1,
                         x0 + buttonSize / 2, y0 + buttonSize - 2);
      } else {
         view->drawLine (getStyle()->color, Color::SHADING_NORMAL,
                         x0 + 1, y0 + 1, x0 + 1, y0 + buttonSize - 2);
         view->drawLine (getStyle()->color, Color::SHADING_NORMAL,
                         x0 + 1, y0 + 1,
                         x0 + buttonSize - 2, y0 + buttonSize / 2);
         view->drawLine (getStyle()->color, Color::SHADING_NORMAL,
                         x0 + 1, y0  + buttonSize - 2,
                         x0 + buttonSize - 2, y0 + buttonSize / 2);
      }
      break;
   }

   DBG_OBJ_LEAVE ();
}

::dw::core::Iterator *Toggle::iterator (Content::Type mask, bool atEnd)
{
   return new ToggleIterator (this, mask, atEnd);
}

bool Toggle::insideButton (MousePositionEvent *event)
{
   int buttonSize = calcButtonSize ();
   return
      event->xWidget >= getStyle()->boxOffsetX () &&
      event->xWidget < getStyle()->boxOffsetX () + buttonSize &&
      event->yWidget >= getAllocation()->ascent - buttonSize &&
      event->yWidget < getAllocation()->ascent;
}

bool Toggle::buttonPressImpl (EventButton *event)
{
   if (event->button == 1 && insideButton (event)) {
      buttonDown = true;
      return true;
   } else
      return false;
}

bool Toggle::buttonReleaseImpl (EventButton *event)
{
   if (event->button == 1 && insideButton (event)) {
      if (buttonDown) {
         showLarge = !showLarge;
         queueResize (0, true);
      }
      return true;
   } else
      return false;
}

bool Toggle::motionNotifyImpl (::dw::core::EventMotion *event)
{
   if ((event->state & BUTTON1_MASK) && !insideButton (event))
      buttonDown = false;
   return false;
}

void Toggle::leaveNotifyImpl (EventCrossing *event)
{
   buttonDown = false;
}

void Toggle::removeChild (Widget *child)
{
   if (child == small)
      small = NULL;
   else if (child == large)
      large = NULL;
   else
      assertNotReached ();
}

void Toggle::setSmall (Widget *widget)
{
   if (small)
      delete small;

   small = widget;
   if (small)
      small->setParent (this);

   if (!showLarge)
      queueResize (0, true);
}

void Toggle::setLarge (Widget *widget)
{
   if (large)
      delete large;

   large = widget;
   if (large)
      large->setParent (this);

   if (showLarge)
      queueResize (0, true);
}

void Toggle::toggle (bool showLarge)
{
   this->showLarge = showLarge;
   queueResize (0, true);
}

} // namespace rtfl

} // namespace dw
