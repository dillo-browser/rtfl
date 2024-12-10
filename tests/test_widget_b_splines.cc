/*
 * RTFL
 *
 * Copyright 2015 Sebastian Geerken <sgeerken@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <FL/Fl_Window.H>
#include <FL/Fl.H>

#include "dw/core.hh"
#include "dw/fltkcore.hh"
#include "dw/fltkviewport.hh"
#include "dwr/tools.hh"

using namespace dw;
using namespace dw::core;
using namespace dw::core::style;
using namespace dw::fltk;
using namespace rtfl::dw::tools;

class BSplineWidget: public Widget
{
public:
   void sizeRequestImpl (Requisition *requisition);
   void getExtremesImpl(Extremes *extremes);
   void draw (View *view, Rectangle *area);
   Iterator *iterator (Content::Type mask, bool atEnd);
};

void BSplineWidget::sizeRequestImpl (Requisition *requisition)
{
   requisition->width = getAvailWidth (true);
   requisition->ascent = getAvailHeight (true);
   requisition->descent = 0;
}

void BSplineWidget::getExtremesImpl(Extremes *extremes)
{
   extremes->minWidth = extremes->maxWidth = extremes->minWidthIntrinsic =
      extremes->maxWidthIntrinsic = 1;
}

void BSplineWidget::draw (View *view, Rectangle *area)
{
   double xp[7] = { 0.4, 0.1, 0.2, 0.5, 0.5, 0.9, 0.7 };
   double yp[7] = { 0.1, 0.5, 0.8, 0.7, 0.2, 0.4, 0.9 };
   int x[7], y[7];

   for (int i = 0; i < 7; i++) {
      x[i] = allocation.x + xp[i] * allocation.width;
      y[i] = allocation.y + yp[i] * getHeight ();
   }

   for(int i = 0; i < 7; i++) {
      view->drawArc (getStyle()->color, core::style::Color::SHADING_NORMAL,
                     true, x[i], y[i], 5, 5, 0, 360);
   }

   drawBSpline (view, getStyle(), 4, 7, x, y);
}

Iterator *BSplineWidget::iterator (Content::Type mask, bool atEnd)
{
   return new core::EmptyIterator (this, mask, atEnd);
}

int main(int argc, char **argv)
{
   FltkPlatform *platform = new FltkPlatform ();
   Layout *layout = new Layout (platform);

   Fl_Window *window = new Fl_Window(500, 400, "Dw Example");
   window->box(FL_NO_BOX);
   window->begin();

   FltkViewport *viewport = new FltkViewport (0, 0, 500, 400);
   layout->attachView (viewport);

   StyleAttrs styleAttrs;
   styleAttrs.initValues ();
   styleAttrs.padding.setVal (5);

   FontAttrs fontAttrs;
   fontAttrs.name = "DejaVu Sans";
   fontAttrs.size = 14;
   fontAttrs.weight = 400;
   fontAttrs.style = FONT_STYLE_NORMAL;
   fontAttrs.letterSpacing = 0;
   fontAttrs.fontVariant = FONT_VARIANT_NORMAL;
   styleAttrs.font = style::Font::create (layout, &fontAttrs);

   styleAttrs.color = Color::create (layout, 0x000000);
   styleAttrs.backgroundColor = Color::create (layout, 0xffffff);

   Style *style = Style::create (&styleAttrs);

   BSplineWidget *widget = new BSplineWidget ();
   widget->setStyle (style);
   layout->setWidget (widget);
   style->unref();

   window->resizable(viewport);
   window->show();
   int errorCode = Fl::run();

   delete layout;

   return errorCode;
}
