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

#include "hideable.hh"

using namespace dw::core;
using namespace dw::core::style;
using namespace lout::misc;

namespace rtfl {

namespace dw {

int Hideable::CLASS_ID = -1;

Hideable::Hideable ()
{
   DBG_OBJ_CREATE ("rtfl::dw::Hideable");
   registerName ("rtfl::dw::Hideable", &CLASS_ID);

   hidden = drawable = false;
}

Hideable::~Hideable ()
{
   DBG_OBJ_DELETE ();
}

void Hideable::sizeRequestImpl (Requisition *requisition)
{
   if (hidden) {
      requisition->width = requisition->ascent = requisition->descent = 0;
      drawable = false;
   } else {
      sizeRequestImplImpl (requisition);
      drawable = true;
   }
}

void Hideable::getExtremesImpl (Extremes *extremes)
{
   if (hidden)
      extremes->minWidth = extremes->minWidthIntrinsic = extremes->maxWidth =
         extremes->maxWidthIntrinsic = 0;
   else
      getExtremesImplImpl (extremes);
}

void Hideable::draw (View *view, Rectangle *area)
{
   DBG_OBJ_ENTER ("draw", 0, "draw", "%d, %d, %d * %d",
                  area->x, area->y, area->width, area->height);

   if (drawable)
      drawImpl (view, area);

   DBG_OBJ_LEAVE ();
}

void Hideable::show ()
{
   if (hidden) {
      hidden = false;
      queueResize (0, true);
   }
}

void Hideable::hide ()
{
   if (!hidden) {
      hidden = true;
      queueResize (0, true);
   }
}

} // namespace rtfl

} // namespace dw
