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

#include "common/fltk_lines.hh"
#include "objcount_window.hh"
#include "objcount_controller.hh"

using namespace rtfl::objects;
using namespace rtfl::common;

int main(int argc, char **argv)
{
   ObjCountWindow *window = new ObjCountWindow(800, 600, "RTFL: Objects count");
   window->show();

   FltkDefaultSource source;
   ObjCountController controller (window->getTable ());
   ObjectsParser parser (&controller);
   source.setup (&parser);

   int errorCode = Fl::run();
   delete window;
   return errorCode;
}
