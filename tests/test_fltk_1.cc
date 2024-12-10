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
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl.H>

Fl_Window *window;
Fl_Menu_Bar *menu;

static void changed (Fl_Widget *widget, void *data);

Fl_Menu_Item menuItems[] = {
      { "&Week",     0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { "Monday", 0, changed, NULL, FL_MENU_TOGGLE, 0, 0, 0, 0 },
      { "Tuesday", 0, changed, NULL, FL_MENU_TOGGLE, 0, 0, 0, 0 },
      { "Wednesday", 0, changed, NULL, FL_MENU_TOGGLE, 0, 0, 0, 0 },
      { "Thursday", 0, changed, NULL, FL_MENU_TOGGLE, 0, 0, 0, 0 },
      { "Friday", 0, changed, NULL, FL_MENU_TOGGLE, 0, 0, 0, 0 },
      { "Saturday", 0, changed, NULL, FL_MENU_TOGGLE, 0, 0, 0, 0 },
      { "Sunday", 0, changed, NULL, FL_MENU_TOGGLE, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
   };

void changed (Fl_Widget *widget, void *data)
{
   printf ("Monday: %s\n",
           menu->find_item("&Week/Monday")->value () ? "set" : "clear");
   printf ("Tuesday: %s\n",
           menu->find_item("&Week/Tuesday")->value () ? "set" : "clear");
   printf ("etc.\n");
}

int main(int argc, char **argv)
{
   window = new Fl_Window(100, 24, "FLTK Test 1");
   window->box(FL_NO_BOX);
   window->begin();

   menu = new Fl_Menu_Bar(0, 0, 100, 24);
   menu->copy(menuItems);

   window->show();
   int errorCode = Fl::run();
   return errorCode;
}
