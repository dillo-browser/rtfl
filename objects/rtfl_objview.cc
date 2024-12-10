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

#include <unistd.h>
#include <FL/Fl.H>
#include "common/fltk_lines.hh"
#include "objview_window.hh"
#include "objview_controller.hh"
#include "objdelete_controller.hh"
#include "objident_controller.hh"

using namespace rtfl::objects;
using namespace rtfl::common;

static void printHelp (const char *argv0)
{
   fprintf
      (stderr, "Usage: %s <options>\n"
       "\n"
       "Options:\n"
       "   -a <aspect>      Show,\n"
       "   -A <aspect>      hide aspects. <aspect> may be '*'.\n"
       "   -b               Do,\n"
       "   -B               do not apply \".rtfl\" and filtering identities "
       "(what \n"
       "                    \"rtfl-objbase\" does).\n"
       "   -m               Show,\n"
       "   -M               hide the messages of all object boxes.\n"
       "   -o               Show,\n"
       "   -O               hide the contents of all object boxes.\n"
       "   -p <prio>        Set priority. <prio> is a number or '*'.\n"
       "   -t <types>       Show,\n"
       "   -T <types>       hide command types. <types> is a sequence of any "
       "of the\n"
       "                    characters 'c', 'i', 'm', 'a', 'f', 's', 't', "
       "'d'.\n"
       "   -v <viewer>      Use <viewer> to view code. Contains '%%p' as "
       "variable for\n"
       "                    the path, and '%%n' for the line number.\n"
       "\n"
       "See RTFL documentation for more details.\n",
       argv0);  
}

static bool toggleCommandTypes (ObjViewWindow *window, const char *arg,
                                bool val)
{
   for (const char *s = arg; *s; s++) {
      switch (*s) {
      case 'c':
         window->showCreateCommands (val);
         break;

      case 'i':
         window->showIndentCommands (val);
         break;

      case 'm':
         window->showMessageCommands (val);
         break;

      case 'a':
         window->showMarkCommands (val);
         break;

      case 'f':
         window->showFunctionCommands (val);
         break;

      case 's':
         window->showAssocCommands (val);
         break;

      case 't':
         window->showAddAttrCommands (val);
         break;

      case 'd':
         window->showDeleteCommands (val);
         break;

      default:
         return false;
      }
   }

   return true;
}

int main(int argc, char **argv)
{
   ObjViewWindow *window = new ObjViewWindow(800, 600, "RTFL: Objects view");

   int opt;
   bool baseFiltering = true;

   while ((opt = getopt(argc, argv, "a:A:bBMmOop:t:T:v:")) != -1) {
      switch (opt) {
      case 'a':
         if (strcmp (optarg, "*") == 0)
            window->setAspectsInitiallySet (true);
         else
            window->addAspect (optarg, true);
         break;

      case 'A':
         if (strcmp (optarg, "*") == 0)
            window->setAspectsInitiallySet (false);
         else
            window->addAspect (optarg, false);
         break;

      case 'b':
         baseFiltering = true;
         break;

      case 'B':
         baseFiltering = false;
         break;

      case 'm':
         window->showObjectMessages (true);
         break;

      case 'M':
         window->showObjectMessages (false);
         break;

      case 'o':
         window->showObjectContents (true);
         break;

      case 'O':
         window->showObjectContents (false);
         break;

      case 'p':
         if (strcmp (optarg, "*") == 0)
            window->setAnyPriority ();
         else 
            window->setPriority (atoi (optarg));
         break;

      case 't':
         if (!toggleCommandTypes (window, optarg, true)) {
            printHelp (argv[0]);
            delete window;
            return 1;
         }
         break;

      case 'T':
         if (!toggleCommandTypes (window, optarg, false)) {
            printHelp (argv[0]);
            delete window;
            return 1;
         }
         break;

      case 'v':
         window->setCodeViewer (optarg);
         break;

      default:
         printHelp (argv[0]);
         delete window;
         return 1;
      }
   }

   int errorCode;
   ObjViewController viewController (window->getObjViewGraph ());
   
   if (baseFiltering)  {
      FltkDefaultSource source;
      ObjIdentController identController (&viewController);
      ObjDeleteController deleteController (&identController);
      ObjectsParser parser (&deleteController);
      source.setup (&parser);
      window->show();
      errorCode = Fl::run();
   } else {
      FltkLinesSource source;
      ObjectsParser parser (&viewController);
      source.setup (&parser);
      window->show();
      errorCode = Fl::run();
   }
   
   delete window;
   return errorCode;
}
