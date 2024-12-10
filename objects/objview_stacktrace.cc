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

#include "objview_stacktrace.hh"

#include "config.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>

namespace rtfl {

namespace objects {

ObjViewStacktraceWindow::ObjViewStacktraceWindow (ObjViewFunction *topFunction,
                                                  ObjViewListener *listener) :
   Fl_Window (WIDTH, HEIGHT, "RTFL: Stack trace")
{
   callback(windowCallback, this);

   browser =
      new Fl_Hold_Browser (SPACE, SPACE, WIDTH - 2 * SPACE,
                             HEIGHT - 3 * SPACE - BUTTON_HEIGHT, NULL);
   Fl_Button *jumpTo =
      new Fl_Button(WIDTH - 2 * BUTTON_WIDTH - 2 * SPACE,
                    HEIGHT - BUTTON_HEIGHT - SPACE, BUTTON_WIDTH,
                    BUTTON_HEIGHT, "Jump to");
   jumpTo->callback (ObjViewStacktraceWindow::jumpTo, this);
   Fl_Return_Button *close =
      new Fl_Return_Button(WIDTH - BUTTON_WIDTH - SPACE, 
                           HEIGHT - BUTTON_HEIGHT - SPACE, BUTTON_WIDTH,
                           BUTTON_HEIGHT, "Close");
   close->callback (ObjViewStacktraceWindow::close, this);

   resizable (browser);

   this->topFunction = topFunction;
   this->listener = listener;
}

ObjViewStacktraceWindow::~ObjViewStacktraceWindow ()
{
}

void ObjViewStacktraceWindow::update ()
{
   browser->clear ();

   for (ObjViewFunction *fun = topFunction; fun;
        fun = fun->getParent ()) {
      char *name = fun->createName ();

      char name2[1024];
      name2[0] = 0;
      
      if (!fun->isSelectable ()) {
         strcpy (name2, "@i");
      }
      
      sprintf (name2 + strlen (name2), "@B%ld@.",
               ((long)fun->getColor ()) << 8);

      char *s = name2 + strlen (name2);
      for (int i = 0; name[i]; i++) {
         if (name[i] == '@') 
            *(s++) = '@';
         *(s++) = name[i];
      }         
      *s = 0;

      browser->add (name2, NULL);
         
      fun->freeName (name);
   }
}

void ObjViewStacktraceWindow::windowCallback (Fl_Widget *widget, void *data)
{
   close (widget, data);
}

void ObjViewStacktraceWindow::jumpTo (Fl_Widget *widget, void *data)
{
   ObjViewStacktraceWindow *win = (ObjViewStacktraceWindow*)data;

   ObjViewFunction *fun = win->topFunction;
   for (int i = 1; fun; fun = fun->getParent (), i++) {
      if (win->browser->selected (i) && fun->isSelectable ())
         fun->select ();
   }
}

void ObjViewStacktraceWindow::close (Fl_Widget *widget, void *data)
{
   ObjViewStacktraceWindow *win = (ObjViewStacktraceWindow*)data;
   win->listener->close (win);
}

} // namespace objects

} // namespace rtfl
