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

#include "about.hh"

#include "config.h"

#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Box.H>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

namespace rtfl {

namespace common {

void AboutWindow::close (Fl_Widget *widget, void *data)
{
   ((AboutWindow*)data)->hide ();
}


AboutWindow::AboutWindow (const char *prgName, const char *licenceException,
                          int height) :
   Fl_Window (WIDTH, height, "")
{

   const char *titleFmt = "RTFL: About %s";
   const char *textFmt =
      "%s " VERSION "\n"
      "\n"
      "%s is part of RTFL (Read The Figurative Logfile).\n"
      "\n"
      "Copyright 2013-2015 Sebastian Geerken <sgeerken@@dillo.org>\n"
      "\n"
      "RTFL is free software; you can redistribute it and/or modify it under "
      "the terms of the GNU General Public License as published by the Free "
      "Software Foundation; either version 3 of the License, or  (at your "
      "option) any later version%s.\n"
      "\n"
      "With RTFL comes some documentation, see “doc/rtfl.html” in the "
      "tarball. For more informations, updates etc. see "
      "<http://home.gna.org/rtfl/>.";

   int titleLen = strlen (titleFmt) - 2 + strlen (prgName) + 1;
   title = new char [titleLen];
   snprintf (title, titleLen, titleFmt, prgName);
   label (title);

   char *capName = strdup (prgName);
   capName[0] = toupper (capName[0]);
   int textLen =
      strlen (textFmt) - 2 + strlen (prgName) - 2 + strlen (capName) + 1
      - 2 + strlen (licenceException);
   text = new char[textLen];
   snprintf (text, textLen, textFmt, prgName, capName, licenceException);
   free (capName);   

   Fl_Box *textWidget =
      new Fl_Box(SPACE, SPACE, WIDTH - 2 * SPACE,
                 height - 3 * SPACE - BUTTON_HEIGHT, text);
   textWidget->box(FL_NO_BOX);
   textWidget->align(FL_ALIGN_WRAP);
   
   Fl_Return_Button *close =
      new Fl_Return_Button(WIDTH - BUTTON_WIDTH - SPACE, 
                           height - BUTTON_HEIGHT - SPACE, BUTTON_WIDTH,
                           BUTTON_HEIGHT, "Close");
   close->callback (AboutWindow::close, this);
}

AboutWindow::~AboutWindow ()
{
   delete[] title;
   delete[] text;
}

} // namespace common

} // namespace rtfl
