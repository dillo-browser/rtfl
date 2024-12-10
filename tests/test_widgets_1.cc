/*
 * RTFL
 *
 * Copyright 2013-2015 Sebastian Geerken <sgeerken@dillo.org>
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

#include "dwr/graph.hh"
#include "dwr/label.hh"
#include "dwr/hbox.hh"
#include "dwr/vbox.hh"
#include "dwr/toggle.hh"

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

   Style *graphStyle = Style::create (&styleAttrs);
   Graph *graph = new Graph ();
   graph->setStyle (graphStyle);
   layout->setWidget (graph);
   graphStyle->unref();

   styleAttrs.borderWidth.setVal (1);
   styleAttrs.setBorderStyle (BORDER_OUTSET);
   styleAttrs.setBorderColor (Color::create (layout, 0x000000));
   Style *textblockStyle = Style::create (&styleAttrs);

   styleAttrs.borderWidth.setVal (0);
   styleAttrs.padding.setVal (0);
   Style *textStyle = Style::create (&styleAttrs);

   graph->setRefStyle (textblockStyle);

   HBox *n1 = new HBox (false);
   n1->setStyle (textblockStyle);
   graph->addNode (n1);   

   Label *l11 = new Label ("Hello <i>w<b>or</i>ld!</b>");
   l11->setStyle (textblockStyle);
   n1->addChild (l11);

   Label *l12 = new Label ("More text ...");
   l12->setStyle (textblockStyle);
   n1->addChild (l12);

   VBox *n2 = new VBox (false);
   n2->setStyle (textblockStyle);
   graph->addNode (n2);

   Label *l21 = new Label ("Ἐν ἀρχῇ ἦν ὁ Λόγος, καὶ ὁ Λόγος ἦν πρὸς τὸν Θεόν, "
                           "καὶ Θεὸς ἦν ὁ Λόγος.");
   l21->setStyle (textblockStyle);
   n2->addChild (l21);

   Label *l22 = new Label ("Οὗτος ἦν ἐν ἀρχῇ πρὸς τὸν Θεόν. πάντα δι' αὐτοῦ "
                           "ἐγένετο, καὶ χωρὶς αὐτοῦ ἐγένετο οὐδὲ ἕν ὃ "
                           "γέγονεν.");
   l22->setStyle (textblockStyle);
   n2->addChild (l22);

   Toggle *n3 = new Toggle (true);
   n3->setStyle (textblockStyle);
   graph->addNode (n3);

   Label *l31 = new Label ("small");
   l31->setStyle (textblockStyle);
   n3->setSmall (l31);

   Label *l33 = new Label ("LLLAAAAARRRRRGGGEEE");
   l33->setStyle (textblockStyle);
   n3->setLarge (l33);
   
   Label *n4 = new Label ("#4");
   n4->setStyle (textblockStyle);
   graph->addNode (n4);

   graph->addEdge (n1, n2);
   graph->addEdge (n1, n3);
   graph->addEdge (n2, n1);

   textStyle->unref();
   textblockStyle->unref();

   window->resizable(viewport);
   window->show();
   int errorCode = Fl::run();

   delete layout;

   return errorCode;
}
