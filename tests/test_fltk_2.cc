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
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl.H>

Fl_Window *window;
Fl_Hold_Browser *browser;

int main(int argc, char **argv)
{
   window = new Fl_Window(500, 500, "FLTK Test 2");
   window->begin();

   browser = new Fl_Hold_Browser (0, 0, 500, 500, NULL);

   for (int n = 0; n < 255; n++) {
      char buf[256];

      snprintf (buf, sizeof(buf), "@B%d@.0x%02x - %d", n, n, n);
      browser->add (buf, NULL);

      snprintf (buf, sizeof(buf), "@i@B%d@.0x%02x - %d", n, n, n);
      browser->add (buf, NULL);
   }     

   window->show();
   int errorCode = Fl::run();
   return errorCode;
}
