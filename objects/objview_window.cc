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

#include <FL/Fl.H>
#include <limits.h>

#include "objview_window.hh"

#include "dw/core.hh"
#include "dw/fltkcore.hh"
#include "dw/fltkviewport.hh"

#include "dwr/graph.hh"

using namespace dw;
using namespace dw::core;
using namespace dw::core::style;
using namespace dw::fltk;
using namespace lout::object;
using namespace lout::container::typed;
using namespace lout::misc;

namespace rtfl {

namespace objects {

ObjViewWindow::Aspect::Aspect (ObjViewWindow *window, const char *name,
                               bool set)
{
   this->window = window;
   this->name = strdup (name);
   this->set = set;
}


ObjViewWindow::Aspect::~Aspect ()
{
   free (name);
}


bool ObjViewWindow::Aspect::equals(Object *other)
{
   Aspect *otherAspect = (Aspect*)other;
   return strcmp (name, otherAspect->name) == 0 &&
      ((set && otherAspect->set) || (!set && !otherAspect->set));
}


int ObjViewWindow::Aspect::compareTo(Comparable *other)
{
   Aspect *otherAspect = (Aspect*)other;
   return strcmp (name, otherAspect->name);
}


// ----------------------------------------------------------------------


ObjViewWindow::Priority::Priority (ObjViewWindow *window, int value)
{
   this->window = window;
   this->value = value;
}


bool ObjViewWindow::Priority::equals(Object *other)
{
   Priority *otherPriority = (Priority*)other;
   return value == otherPriority->value;
}


int ObjViewWindow::Priority::compareTo(Comparable *other)
{
   Priority *otherPriority = (Priority*)other;
   return value - otherPriority->value;
}


// ----------------------------------------------------------------------


ObjViewWindow::ObjViewWindow (int width, int height, const char *title):
   Fl_Window (width, height, title)
{
   shown = false;

   aspects = new HashTable<String, Aspect> (true, true);
   aspectsMenuPositions = new Vector<Integer> (1, true);;
   aspectsInitiallySet = true;

   priorities = new HashTable<Integer, Priority> (true, true);
   prioritiesMenuPositions = new Vector<Integer> (1, true);;
   selectedPriority = INT_MAX;

   // Special priority "no priority" (INT_MAX):
   Priority *noLimitsPriority = new Priority (this, INT_MAX);
   priorities->put (new Integer (INT_MAX), noLimitsPriority);
   
   aboutWindow = NULL;

   int menuHeight = 24;

   FltkPlatform *platform = new FltkPlatform ();
   layout = new Layout (platform);

   callback(windowCallback, NULL);
   box(FL_NO_BOX);

   menu = new Fl_Menu_Bar(0, 0, width, menuHeight);

   FltkViewport *viewport =
      new FltkViewport (0, menuHeight, width, height - menuHeight);
   layout->attachView (viewport);

   graph = new ObjViewGraph (this);
   layout->setWidget (graph);
   graph->initStyles ("DejaVu Sans", 12, 0x000000, 0xffffff, 0xffffd0, 1, 10);

   Fl_Menu_Item menuItems[] = {
      { "&File",     0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { "Quit", FL_COMMAND + 'q', quit, this, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { "&Command", 0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { "&Previous", FL_COMMAND + 'p', previous, this, 0, 0, 0, 0, 0 },
      { "&Next", FL_COMMAND + 'n', next, this, 0, 0, 0, 0, 0 },
      { "View &Code", FL_COMMAND + 'c', viewCode, this, FL_MENU_DIVIDER,
        0, 0, 0, 0 },
      { "Hide &before", FL_COMMAND + 'b', hideBefore, this, 0, 0, 0, 0, 0 },
      { "Hide &after", FL_COMMAND + 'a', hideAfter, this, 0, 0, 0, 0, 0 },
      { "&Hide all", FL_COMMAND + 'h', hideAll, this, FL_MENU_DIVIDER,
        0, 0, 0, 0 },
      { "Show b&efore", FL_COMMAND + 'e', showBefore, this, 0, 0, 0, 0, 0 },
      { "Show a&fter", FL_COMMAND + 'f', showAfter, this, 0, 0, 0, 0, 0 },
      { "&Show all", FL_COMMAND + 's', showAll, this, FL_MENU_DIVIDER,
        0, 0, 0, 0 },
      { "Show stack trace", FL_COMMAND + 't', showStackTrace, this, 0, 0, 0, 0,
        0 },
      { "Switch between related", FL_COMMAND + 'r', switchBetweenRelated, this,
        FL_MENU_DIVIDER, 0, 0, 0, 0 },
      { "&Creations", 0, toggleCommandTypeVisibility, this, FL_MENU_TOGGLE,
        0, 0, 0, 0 },
      { "&Indentations", 0, toggleCommandTypeVisibility, this, FL_MENU_TOGGLE,
        0, 0, 0, 0 },
      { "&Messages", 0, toggleCommandTypeVisibility, this,
        FL_MENU_TOGGLE | FL_MENU_VALUE, 0, 0, 0, 0 },
      { "M&arks", 0, toggleCommandTypeVisibility, this,
        FL_MENU_TOGGLE | FL_MENU_VALUE, 0, 0, 0, 0 },
      { "&Functions", 0, toggleCommandTypeVisibility, this,
        FL_MENU_TOGGLE | FL_MENU_VALUE, 0, 0, 0, 0 },
      { "A&ssociations", 0, toggleCommandTypeVisibility, this,
        FL_MENU_TOGGLE | FL_MENU_VALUE, 0, 0, 0, 0 },
      { "A&ttributes", 0, toggleCommandTypeVisibility, this,
        FL_MENU_TOGGLE | FL_MENU_VALUE, 0, 0, 0, 0 },
      { "&Deletions", 0, toggleCommandTypeVisibility, this,
        FL_MENU_TOGGLE | FL_MENU_VALUE, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { "&Aspects", 0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { "&Show all", 0, showAllAspects, this, FL_MENU_TOGGLE | FL_MENU_VALUE,
        0, 0, 0, 0 },
      { "&Hide all", 0, hideAllAspects, this, FL_MENU_TOGGLE | FL_MENU_DIVIDER,
        0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { "&Priorities", 0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { "&No limit", 0, setPriority, noLimitsPriority,
        FL_MENU_RADIO | FL_MENU_VALUE /*| FL_MENU_DIVIDER*/, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { "&Marks", 0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { "&Help", 0, 0, 0, FL_SUBMENU, 0, 0, 0, 0 },
      { "&About RTFL", 0, about, this, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },

      { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
   };
   menu->copy(menuItems);

   noLimitsPriority->menuItem =
      (Fl_Menu_Item*)(menu->find_item ("&Priorities/&No limit"));
   
   resizable(viewport);
}

ObjViewWindow::~ObjViewWindow ()
{
   delete aspects;
   delete aspectsMenuPositions;
   delete priorities;
   delete prioritiesMenuPositions;
   if (aboutWindow)
      delete aboutWindow;
   delete layout;
}


void ObjViewWindow::show ()
{
   shown = true;

   Integer key (selectedPriority);
   Priority *priorityEntry = priorities->get (&key);
   priorityEntry->menuItem->activate ();

   Fl_Window::show ();
}

void ObjViewWindow::addAspect (const char *aspect)
{
   addAspect (aspect, aspectsInitiallySet);
}

void ObjViewWindow::setAspectsInitiallySet (bool val)
{
   aspectsInitiallySet = val;
   showOrHideAllAspects (aspectsInitiallySet);

   if (aspectsInitiallySet) {
      getShowAllAspectsMenuItem()->set ();
      getHideAllAspectsMenuItem()->clear ();
   } else {
      getShowAllAspectsMenuItem()->clear ();
      getHideAllAspectsMenuItem()->set ();
   }
}

void ObjViewWindow::addPriority (int priority)
{
   addPriority (priority, false);
}


bool ObjViewWindow::isAspectSelected (const char *aspect)
{
   String key (aspect);
   Aspect *aspectEntry = aspects->get (&key);
   assert (aspectEntry != NULL); // Should have been added before.
   return aspectEntry->set;
}


bool ObjViewWindow::isPrioritySelected (int priority)
{
   return priority <= selectedPriority;
}


bool ObjViewWindow::isTypeSelected (OVGCommandType type)
{
   switch (type) {
   case OVG_COMMAND_CREATE:
      return getCreateMenuItem()->value ();

   case OVG_COMMAND_INDENT:
      return getIndentMenuItem()->value ();

   case OVG_COMMAND_MESSAGE:
      return getMessageMenuItem()->value ();

   case OVG_COMMAND_MARK:
      return getMarkMenuItem()->value ();

   case OVG_COMMAND_FUNCTION:
      return getFunctionMenuItem()->value ();

   case OVG_COMMAND_ASSOC:
      return getAssocMenuItem()->value ();

   case OVG_COMMAND_ADD_ATTR:
      return getAddAttrMenuItem()->value ();

   case OVG_COMMAND_DELETE:
      return getDeleteMenuItem()->value ();

   }

   assertNotReached ();
   return false;
}

void ObjViewWindow::addMark (OVGAddMarkCommand *markCommand)
{
   int pathLen = 6 + strlen (markCommand->getMark ()) + 1;
   char *path = new char[pathLen];
   snprintf (path, pathLen, "Marks/%s", markCommand->getMark ());
   int menuPos = menu->add (path, 0, NULL, NULL, 0);
   delete[] path;      
   Fl_Menu_Item *menuItem = (Fl_Menu_Item*)(menu->menu() + menuPos);
   menuItem->callback (jumpToMark, markCommand);
}


void ObjViewWindow::addAspect (const char *aspect, bool val)
{
   String key (aspect);
   Aspect *aspectEntry = aspects->get (&key);
   if (aspectEntry == NULL) {
      aspectEntry = new Aspect (this, aspect, val);
      aspects->put (new String (aspect), aspectEntry);

#if 1
      int pathLen = 8 + strlen (aspect) + 1;
      char *path = new char[pathLen];
      snprintf (path, pathLen, "Aspects/%s", aspect);
      int menuPos =
         menu->add (path, 0, NULL, NULL,
                    FL_MENU_TOGGLE | (aspectEntry->set ? FL_MENU_VALUE : 0));
      delete[] path;      
      aspectEntry->menuItem = (Fl_Menu_Item*)(menu->menu() + menuPos);
      aspectEntry->menuItem->callback (toggleAspect, aspectEntry);

      // TODO aspectsMenuPositions is only used in the deactivated
      // code.
#else
      // Aspects are sorted alphabetically, so we choose the following
      // approach:
      //
      // 1. Add a new entry, which is not used for the new aspect, but
      //    for the last aspect after sorting. For this reason, a
      //    random name is choosen.
      //
      // 2. Sort all aspects.
      // 
      // 3. Re-assign all aspects to the menu entries.

      // 1. Add new entry.
      //
      // It seams that a menu entry must have a unique name, so it is
      // important to choose a temporary one which does not collide
      // with an aspect. This one is the result of:
      //
      // $ head -c 32 < /dev/random | base64 | sed 's/\///g'
      //
      // (Further checks are not done, hoping that noone chooses this
      // as an aspect.)

      int menuPos =
         menu->add ("Aspects/xgIUCOm3HTvK5P33Dsk3lPSL1qgXg4Iye3APe0ckMo0=",
                    0, NULL, NULL, FL_MENU_TOGGLE);
      aspectsMenuPositions->put (new Integer (menuPos));

      // 2. Sort all aspects.
      
      Vector<Aspect> sortedAspects (1, false);
      for (lout::container::typed::Iterator<String> it = aspects->iterator ();
           it.hasNext (); ) {
         String *key = it.getNext ();
         Aspect *value = aspects->get (key);
         sortedAspects.insertSorted (value);
      }

      // 3. Re-assign all aspects to the menu entries.

      assert (sortedAspects.size () == aspectsMenuPositions->size ());

      for (int i = 0; i < sortedAspects.size (); i++) {
         Aspect *aspect = sortedAspects.get (i);
         int menuPos = aspectsMenuPositions->get(i)->getValue ();

         // This is somewhat ugly (ignoring "const"):
         aspect->menuItem = (Fl_Menu_Item*)(menu->menu() + menuPos);
         //aspect->menuItem->label (aspect->name);
         menu->replace (menuPos, aspect->name);
         if (aspect->set)
            aspect->menuItem->set ();
         else
               aspect->menuItem->clear ();
         aspect->menuItem->callback (toggleAspect, aspect);
      }
#endif
   }
}


ObjViewWindow::Priority *ObjViewWindow::addPriority (int priority, bool val)
{
   // Similar to addAspect().

   Integer key (priority);
   Priority *priorityEntry = priorities->get (&key);
   if (priorityEntry == NULL) {
      priorityEntry = new Priority (this, priority);
      priorities->put (new Integer (priority), priorityEntry);

#if 1
      int pathLen = 11 + 5 + 1;
      char path[pathLen];
      snprintf (path, pathLen, "Priorities/%d", priority);
      int menuPos = menu->add (path, 0, NULL, NULL,
                               FL_MENU_RADIO | (val ? FL_MENU_VALUE : 0));
      priorityEntry->menuItem = (Fl_Menu_Item*)(menu->menu() + menuPos);
      priorityEntry->menuItem->callback (setPriority, priorityEntry);

      // TODO prioritiesMenuPositions is only used in the deactivated
      // code.
#else
      // 1. Add new entry.

      int menuPos =
         menu->add ("Priorities/xgIUCOm3HTvK5P33Dsk3lPSL1qgXg4Iye3APe0ckMo0=",
                    0, NULL, NULL, FL_MENU_RADIO);
      prioritiesMenuPositions->put (new Integer (menuPos));

      // 2. Sort all priorities.
      
      Vector<Priority> sortedPriorities (1, false);
      for (lout::container::typed::Iterator<Integer> it =
              priorities->iterator (); it.hasNext (); ) {
         Integer *key = it.getNext ();
         Priority *value = priorities->get (key);
         // "No limits" entry is not sorted.
         if (value->value != INT_MAX)
            sortedPriorities.insertSorted (value);
      }

      // 3. Re-assign all priorities to the menu entries (except "No limits").

      assert (sortedPriorities.size () == prioritiesMenuPositions->size ());

      for (int i = 0; i < sortedPriorities.size (); i++) {
         Priority *priority = sortedPriorities.get (i);
         int menuPos = prioritiesMenuPositions->get(i)->getValue ();

         // Assuming maximal priorities of 99999 (snprintf will at
         // least prevent buffer overflows).
         snprintf (priority->valueBuf, 6, "%d", priority->value);

         // This is somewhat ugly (ignoring "const"):
         priority->menuItem = (Fl_Menu_Item*)(menu->menu() + menuPos);
         //priority->menuItem->label (priority->valueBuf);
         menu->replace (menuPos, priority->valueBuf);
         if (priority->value == selectedPriority)
            priority->menuItem->set ();
         else
            priority->menuItem->clear ();
         priority->menuItem->callback (setPriority, priority);
      }
#endif
   }

   return priorityEntry;
}

void ObjViewWindow::setPriority (int priority)
{
   assert (!shown);

   selectedPriority = priority;
   addPriority (priority, true);
   graph->recalculateCommandsVisibility ();
}


void ObjViewWindow::setAnyPriority ()
{
   assert (!shown);

   selectedPriority = INT_MAX;
   graph->recalculateCommandsVisibility ();
}



void ObjViewWindow::quit (Fl_Widget *widget, void *data)
{
   exit (0);
}


void ObjViewWindow::previous (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->previousCommand ();
}


void ObjViewWindow::next (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->nextCommand ();
}


void ObjViewWindow::viewCode (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->viewCodeOfCommand ();
}


void ObjViewWindow::hideBefore (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->hideBeforeCommand ();
}


void ObjViewWindow::hideAfter (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->hideAfterCommand ();
}


void ObjViewWindow::hideAll (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->hideAllCommands ();
}


void ObjViewWindow::showBefore (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->showBeforeCommand ();
}


void ObjViewWindow::showAfter (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->showAfterCommand ();
}


void ObjViewWindow::showAll (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->showAllCommands ();
}

void ObjViewWindow::showStackTrace (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->showStackTraceOfCommand ();
}

void ObjViewWindow::switchBetweenRelated (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->switchBetweenRelatedCommands ();
}

void ObjViewWindow::toggleCommandTypeVisibility (Fl_Widget *widget, void *data)
{
   ((ObjViewWindow*)data)->graph->recalculateCommandsVisibility ();
}


void ObjViewWindow::showAllAspects (Fl_Widget *widget, void *data)
{
   ObjViewWindow *objectsWindow = (ObjViewWindow*)data;
   bool showSet = objectsWindow->getShowAllAspectsMenuItem()->value ();
   objectsWindow->showOrHideAllAspects (showSet);

   if (showSet)
      objectsWindow->getHideAllAspectsMenuItem()->clear ();
   else
      objectsWindow->getHideAllAspectsMenuItem()->set ();

   objectsWindow->aspectsInitiallySet = true;
}


void ObjViewWindow::hideAllAspects (Fl_Widget *widget, void *data)
{
   ObjViewWindow *objectsWindow = (ObjViewWindow*)data;
   bool hideSet = objectsWindow->getHideAllAspectsMenuItem()->value ();
   objectsWindow->showOrHideAllAspects (!hideSet);

   if (hideSet)
      objectsWindow->getShowAllAspectsMenuItem()->clear ();
   else
      objectsWindow->getShowAllAspectsMenuItem()->set ();

   objectsWindow->aspectsInitiallySet = false;
}


void ObjViewWindow::showOrHideAllAspects (bool value)
{
   for (lout::container::typed::Iterator<String> it = aspects->iterator ();
        it.hasNext (); ) {
      String *key = it.getNext ();
      Aspect *aspect = aspects->get (key);
      aspect->set = value;
      if (value)
         aspect->menuItem->set ();
      else
         aspect->menuItem->clear ();
   }
   
   graph->recalculateCommandsVisibility ();
}


void ObjViewWindow::toggleAspect (Fl_Widget *widget, void *data)
{
   Aspect *aspect = (Aspect*)data;
   aspect->set = aspect->menuItem->value ();

   bool allShown = true, allHidden = true;
   for (lout::container::typed::Iterator<String> it =
           aspect->window->aspects->iterator (); it.hasNext (); ) {
      String *key = it.getNext ();
      Aspect *value = aspect->window->aspects->get (key);
      if (value->set)
         allHidden = false;
      else
         allShown = false;
   }

   // There is at least one entry; otherwise, this method would not be
   // called. (With no elements, allShown && allHidden would always be
   // true.)
   assert (!(allShown && allHidden));

   // If neither all shown, nor all hidden, aspectsInitiallySet is not
   // changed.
   
   if (allShown) {
      aspect->window->getShowAllAspectsMenuItem()->set ();
      aspect->window->aspectsInitiallySet = true;
   } else
      aspect->window->getShowAllAspectsMenuItem()->clear ();

   if (allHidden) {
      aspect->window->getHideAllAspectsMenuItem()->set ();
      aspect->window->aspectsInitiallySet = false;
   } else
      aspect->window->getHideAllAspectsMenuItem()->clear ();

   aspect->window->graph->recalculateCommandsVisibility ();
}


void ObjViewWindow::setPriority (Fl_Widget *widget, void *data)
{
   Priority *priority = (Priority*)data;
   priority->window->selectedPriority = priority->value;
   priority->window->graph->recalculateCommandsVisibility ();
}

void ObjViewWindow::jumpToMark (Fl_Widget *widget, void *data)
{
   OVGAddMarkCommand *markCommand = (OVGAddMarkCommand*)data;
   if (markCommand->isSelectable ())
      markCommand->select ();
}

void ObjViewWindow::about (Fl_Widget *widget, void *data)
{
   ObjViewWindow *window = (ObjViewWindow*)data;

   if (window->aboutWindow == NULL)
      window->aboutWindow =
         new common::AboutWindow("rtfl-objview",
                                 "; with the following exception:\n"
                                 "\n"
                                 "The copyright holders of RTFL give you "
                                 "permission to link rtfl-objview "
                                 "statically or dynamically against all "
                                 "versions of the graphviz library, which are "
                                 "published by AT&T Corp. under one of the "
                                 "following licenses:\n"
                                 "\n"
                                 "- Common Public License version 1.0 as "
                                 "published by International Business Machines "
                                 " Corporation (IBM), or\n"
                                 "- Eclipse Public License version 1.0 as "
                                 "published by the Eclipse Foundation",
                                 common::AboutWindow::HEIGHT_EXCEPTION);
   window->aboutWindow->show ();
}


void ObjViewWindow::windowCallback (Fl_Widget *widget, void *data)
{
   // Ignore escape key. TODO Looks rather hackish to me.
   if (Fl::event_key() != FL_Escape)
      quit (widget, data);
}


} // namespace objects

} // namespace rtfl
