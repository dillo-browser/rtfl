/*
 * RTFL
 *
 * Copyright 2014, 2015 Sebastian Geerken <sgeerken@dillo.org>
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

#include "dwr/vbox.hh"
#include "dwr/label.hh"

using namespace dw;
using namespace dw::core;
using namespace dw::core::style;
using namespace dw::fltk;

using namespace rtfl::dw;

int main(int argc, char **argv)
{
   FltkPlatform *platform = new FltkPlatform ();
   Layout *layout = new Layout (platform);

   Fl_Window *window = new Fl_Window(500, 400, "Dw Example");
   window->box(FL_NO_BOX);
   window->begin();

   FltkViewport *viewport = new FltkViewport (0, 0, 500, 400);
   layout->attachView (viewport);

   FontAttrs fontAttrs;
   fontAttrs.name = "DejaVu Serif";
   fontAttrs.size = 14;
   fontAttrs.weight = 400;
   fontAttrs.style = FONT_STYLE_NORMAL;
   fontAttrs.letterSpacing = 0;
   fontAttrs.fontVariant = FONT_VARIANT_NORMAL;

   StyleAttrs styleAttrs;
   styleAttrs.font = style::Font::create (layout, &fontAttrs);
   styleAttrs.initValues ();
   styleAttrs.margin.setVal (5);
   styleAttrs.borderWidth.setVal (1);
   styleAttrs.setBorderStyle (BORDER_OUTSET);
   styleAttrs.padding.setVal (5);
   styleAttrs.setBorderColor (Color::create (layout, 0x000000));
   styleAttrs.color = Color::create (layout, 0x000000);

   Style *boxStyle = Style::create (&styleAttrs);

   styleAttrs.margin.setVal (0);
   styleAttrs.borderWidth.setVal (0);
   styleAttrs.padding.setVal (0);
   styleAttrs.setBorderColor (Color::create (layout, 0x000000));
   Style *labelStyle = Style::create (&styleAttrs);

   VBox *box = new VBox (false);
   box->setStyle (boxStyle);
   layout->setWidget (box);

   for (int i = 0; i < 1000; i++) {
      char buf[32];
      sprintf (buf, "Label №%d", i);
      Label *label = new Label (buf);
      label->setStyle (labelStyle);
      box->addChild (label, i / 10);
   }

   boxStyle->unref();
   labelStyle->unref();

   window->resizable(viewport);
   window->show();
   int errorCode = Fl::run();

   delete layout;

   return errorCode;
}
