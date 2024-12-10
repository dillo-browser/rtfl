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

#include "objcount_window.hh"
#include "common/about.hh"

#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_draw.H>

using namespace lout::container::typed;
using namespace lout::object;
using namespace lout::misc;

namespace rtfl {

namespace objects {

ObjCountTable::Class::Class (const char *name)
{
   this->name = strdup (name);
   count = new lout::misc::SimpleVector<int> (1);
   count->increase ();
   *(count->getLastRef()) = 0;
}


ObjCountTable::Class::~Class ()
{
   free (name);
   delete count;
}


int ObjCountTable::Class::compareTo(Comparable *other)
{
   return strcmp (name, ((Class*)other)->name);
}


void ObjCountTable::Class::create ()
{
   (*(count->getLastRef()))++;
}


void ObjCountTable::Class::remove ()
{
   (*(count->getLastRef()))--;
}


void ObjCountTable::Class::newSnapshot ()
{
   int c = *(count->getLastRef());
   count->increase ();
   *(count->getLastRef()) = c;
}


// ----------------------------------------------------------------------


int ObjCountTable::Object::classSernoGlobal = 0;


ObjCountTable::Object::Object (Class *klass)
{
   setClass (klass);
   refCount = 0;
}


ObjCountTable::Object::~Object ()
{
}


void ObjCountTable::Object::setClass (Class *klass)
{
   this->klass = klass;
   classSerno = classSernoGlobal++;
}


// ----------------------------------------------------------------------


ObjCountTable::ObjectRef::ObjectRef (rtfl::objects::ObjCountTable::Object
                                     *object)
{
   this->object = object;
   object->ref ();
}


ObjCountTable::ObjectRef::~ObjectRef ()
{
   object->unref ();
}


// ----------------------------------------------------------------------


ObjCountTable::ObjCountTable (int x, int y, int width, int height,
                              const char *label) :
   Fl_Table (x, y, width, height, label)
{
   rows (0);
   row_header (true);
   row_height_all (20);
   row_resize (false);

   cols (1);
   col_header (false);
   row_header_width (200);
   col_width_all (80);
   col_resize (true);

   end();

   objects = new HashTable<String, ObjectRef> (true, true);
   identities = new HashTable<String, String> (true, true);
   identitiesRev = new HashTable<String, String> (false, false);
   classes = new HashTable<String, Class> (true, false);
   classesList = new Vector<Class> (1, true);

   ensureClass ("<unknown>");
}


ObjCountTable::~ObjCountTable()
{
   delete objects;
   delete identitiesRev;
   delete identities;
   delete classes;
   delete classesList;
}


void ObjCountTable::draw_cell (TableContext context, int row, int col, int x,
                               int y, int width, int height)
{
   switch (context) {
   case CONTEXT_COL_HEADER:
      break;

   case CONTEXT_ROW_HEADER:
      fl_push_clip (x, y, width, height);
      fl_draw_box (FL_THIN_UP_BOX, x, y, width, height, row_header_color ());
      fl_color (FL_BLACK);
      fl_draw (classesList->get(row)->name, x, y, width, height, FL_ALIGN_LEFT);
      fl_pop_clip ();
      break;

   case CONTEXT_CELL:
      fl_push_clip (x, y, width, height);
      fl_color (FL_WHITE);
      fl_rectf (x, y, width, height);
      fl_color (FL_BLACK);
      char buf[6];
      snprintf (buf, 6, "%d", classesList->get(row)->count->get (col));
      fl_draw (buf, x, y, width, height, FL_ALIGN_RIGHT);
      fl_pop_clip ();
      break;

   default:
      // compiler happyness
      break;
   }   
}


ObjCountTable::Class *ObjCountTable::ensureClass (const char *className)
{
   String key (className);
   Class *klass = classes->get (&key);

   if (klass == NULL) {
      klass = new Class (className);
      classes->put (new String (className), klass);

      int i = classesList->bsearch (klass, false);
      classesList->insert (klass, i);
      for (int j = i; j < classesList->size (); j++)
         classesList->get(j)->index = j;

      rows (rows () + 1);
   }

   return klass;
}

void ObjCountTable::createObject (const char *id, const char *className)
{
   Class *klass = ensureClass (className);

   String key (id);
   ObjectRef *objectRef = objects->get (&key);

   if (objectRef != NULL) {
      objectRef->object->getClass()->remove ();
      objectRef->object->setClass (klass);
   } else {
      rtfl::objects::ObjCountTable::Object *object;
      String *id2 = identities->get (&key);
      if (id2) {
         ObjectRef *objRef2 = objects->get (id2);
         assert (objRef2 != NULL);
         object = objRef2->object;
      } else
         object = new rtfl::objects::ObjCountTable::Object (klass);
         
      objectRef = new ObjectRef (object);
      objects->put (new String (id), objectRef);
   }

   klass->create ();
   damage_zone(klass->index, cols () - 1, klass->index, cols () - 1);
}


void ObjCountTable::deleteObject (const char *id)
{
   String key (id);
   ObjectRef *objectRef = objects->get (&key);
   if (objectRef != NULL) {
      objectRef->object->getClass()->remove ();
      damage_zone(objectRef->object->getClass()->index,cols () - 1,
                  objectRef->object->getClass()->index, cols () - 1);

      objects->remove (&key);

      String *key2 = identities->get (&key);
      if (key2) {
         identitiesRev->remove (key2);
         identities->remove (&key);
      } else {
         key2 = identitiesRev->get (&key);
         if (key2) {
            identitiesRev->remove (&key);
            identities->remove (key2);
         }
      }
   }
}


void ObjCountTable::registerObject (const char *id)
{
   String key (id);
   if (!objects->contains (&key))
      createObject (id, "<unknown>");
}


void ObjCountTable::addIdentity (const char *id1, const char *id2)
{
   if (strcmp (id1, id2) != 0) {
      // Note: An ObjectRef (which is != NULL) points always to an Object.
      String key1 (id1), key2 (id2);
      ObjectRef *objRef1 = objects->get (&key1),
         *objRef2 = objects->get (&key2);

      if (objRef1 == NULL && objRef2 == NULL) {
         // Neither defined: create both.
         registerObject (id1);
         insertIdentity (id2, id1);
         registerObject (id2);
      } else if (objRef1 == NULL) {
         // First not defined, but second: create from second.
         insertIdentity (id2, id1);
         registerObject (id2);
      } else if (objRef2 == NULL) {
         // Vice versa.
         insertIdentity (id1, id2);
         registerObject (id1);
      } else {
         // Both already defined ...
         if (objRef1->object == objRef2->object)
            // ... for same object: caller's fault.
            fprintf (stderr, "WARNING: Identity of '%s' and '%s' added twice.",
                     id1, id2);
         else {
            // ... for different objects.
            if (objRef1->object->getClassSerno () >
                objRef2->object->getClassSerno ()) {
               // Class definition of first object more recent, so assign it to
               // second.
               objRef2->object->getClass()->remove ();
               damage_zone(objRef2->object->getClass()->index,cols () - 1,
                           objRef2->object->getClass()->index, cols () - 1);
               objRef2->object->unref ();
               objRef2->object = objRef1->object;
               objRef2->object->ref ();
            } else {
               // Vice versa.
               objRef1->object->getClass()->remove ();
               damage_zone(objRef1->object->getClass()->index,cols () - 1,
                           objRef1->object->getClass()->index, cols () - 1);
               objRef1->object->unref ();
               objRef1->object = objRef2->object;
               objRef1->object->ref ();
            }
         }
      }
   }
}


void ObjCountTable::insertIdentity (const char *id1, const char *id2)
{
   String *s1 = new String (id1), *s2 = new String (id2);
   identities->put (s1, s2);
   identitiesRev->put (s2, s1);  
}


void ObjCountTable::setClassColor (const char *klass, const char *color)
{
}


void ObjCountTable::newSnapshot ()
{
   for (int i = 0; i < classesList->size (); i++)
      classesList->get(i)->newSnapshot ();

   cols (cols () + 1);
}


void ObjCountTable::removeOldestSnapshot ()
{
}


// ----------------------------------------------------------------------


ObjCountWindow::ObjCountWindow (int width, int height, const char *title) :
   Fl_Window (width, height, title)
{
   int menuHeight = 24;

   callback(windowCallback, NULL);
   box(FL_NO_BOX);

   Fl_Menu_Bar *menu = new Fl_Menu_Bar(0, 0, width, menuHeight);

   table = new ObjCountTable (0, menuHeight, width, height - menuHeight);

   Fl_Menu_Item menuItems[] = {
      { "&File",     0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { "Quit", FL_COMMAND + 'q', quit, this, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { "&Snapshot", 0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { "New &snapshot", FL_COMMAND + 's', newSnapshot, this, 0, 0, 0, 0, 0 },
      { "&Delete oldest", FL_COMMAND + 'd', removeOldestSnapshot, this,
        0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { "&Help", 0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { "&About RTFL", 0, about, this, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
   };
   menu->copy(menuItems);

   resizable(table);

   aboutWindow = NULL;
}


ObjCountWindow::~ObjCountWindow ()
{
   if (aboutWindow)
      delete aboutWindow;
}


void ObjCountWindow::windowCallback (Fl_Widget *widget, void *data)
{
   // Ignore escape key. TODO Looks rather hackish to me.
   if (Fl::event_key() != FL_Escape)
      quit (widget, data);
}


void ObjCountWindow::quit (Fl_Widget *widget, void *data)
{
   exit (0);
}


void ObjCountWindow::newSnapshot (Fl_Widget *widget, void *data)
{
   ObjCountWindow *window = (ObjCountWindow*)data;
   window->table->newSnapshot ();
}


void ObjCountWindow::removeOldestSnapshot (Fl_Widget *widget, void *data)
{
   ObjCountWindow *window = (ObjCountWindow*)data;
   window->table->removeOldestSnapshot ();
}


void ObjCountWindow::about (Fl_Widget *widget, void *data)
{
   ObjCountWindow *window = (ObjCountWindow*)data;

   if (window->aboutWindow == NULL)
      window->aboutWindow =
         new common::AboutWindow("rtfl-objcount", "",
                                 common::AboutWindow::HEIGHT_SIMPLE);
   window->aboutWindow->show ();
}


} // namespace objects

} // namespace rtfl
