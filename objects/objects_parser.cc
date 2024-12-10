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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "objects_parser.hh"

#if 0
#   define PRINT(fmt) printf ("---- [%p] " fmt "\n", this)
#   define PRINTF(fmt, ...) printf ("---- [%p] " fmt "\n", this, __VA_ARGS__)
#else
#   define PRINT(fmt)
#   define PRINTF(fmt, ...)
#endif

using namespace rtfl::tools;

namespace rtfl {

namespace objects {

namespace timeout {
   
inline int getActualType (int type)
{
   return type >> 8;
}

inline int getCount (int type)
{
   return type & 0xff;
}

inline int makeType (int actualType, int count)
{
   return count | (actualType << 8);
}

inline int incType (int type)
{
   return makeType (getActualType (type), getCount (type) + 1);
}

inline int decType (int type)
{
   return makeType (getActualType (type), getCount (type) - 1);
}

} // namespace timeout

ObjectsControllerBase::ObjectsControllerBase()
{
   predessor = NULL;
   successor = NULL;
}

void ObjectsControllerBase::addTimeout (double secs, int type)
{
   PRINTF ("ObjectsControllerBase::addTimeout (%g, %d)", secs, type);

   if (predessor)
      predessor->addTimeout (secs, timeout::incType (type));
   else
      fprintf (stderr, "addTimeout (%g, %d): no predessor\n", secs, type); 
}

void ObjectsControllerBase::removeTimeout (int type)
{
   if (predessor)
      predessor->removeTimeout (timeout::incType (type));
   else
      fprintf (stderr, "removeTimeout (%d): no predessor\n", type); 
}

void ObjectsControllerBase::setObjectsSource (ObjectsSource *source)
{
   predessor = source;
}

void ObjectsControllerBase::timeout (int type)
{
   PRINTF ("ObjectsControllerBase::timeout (%d)", type);

   // We start sending count == 1 to the predessor, so we get back count == 1
   // from it at the end.
   if (timeout::getCount (type) == 1)
      ownTimeout (timeout::getActualType (type));
   else {
      if (successor)
         successor->timeout (timeout::decType (type));
      else
         fprintf (stderr, "timeout (%d): no successor\n", type);
   }
}

void ObjectsControllerBase::finish ()
{
   ownFinish ();
   
   if (successor)
      successor->finish ();
}

void ObjectsControllerBase::setObjectsSink (ObjectsSink *sink)
{
   successor = sink;
}

void ObjectsControllerBase::addOwnTimeout (double secs, int type)
{
   PRINTF ("ObjectsControllerBase::addOwnTimeout (%g, %d)", secs, type);
   addTimeout (secs, timeout::makeType (0, type));
}

void ObjectsControllerBase::removeOwnTimeout (int type)
{
   removeTimeout (timeout::makeType (0, type));
}
   
void ObjectsControllerBase::ownTimeout (int type)
{
   // No implementation.
}

void ObjectsControllerBase::ownFinish ()
{
   // No implementation.
}

// ----------------------------------------------------------------------

ObjectsParser::ObjectsParser (ObjectsController *controller)
{
   this->controller = controller;
   source = NULL;
   controller->setObjectsSource (this);
}

ObjectsParser::~ObjectsParser ()
{
   controller->setObjectsSource (NULL);
}

void ObjectsParser::processCommand (CommonLineInfo *info, char *cmd, char *args)
{
   char **parts = NULL;

   if (args == NULL)
      // All commands need arguments here.
      fprintf (stderr, "Missing arguments:%s\n", info->completeLine);
   else if (strcmp (cmd, "obj-msg") == 0) {
      parts = split (args, 4);
      if (parts[1] && parts[2] && parts[3])
         controller->objMsg (info, parts[0], parts[1], atoi(parts[2]),
                             parts[3]);
      else
         fprintf (stderr, "Incomplete line (obj-msg):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-mark") == 0) {
      parts = split (args, 4);
      if (parts[1] && parts[2] && parts[3])
         controller->objMark (info, parts[0], parts[1], atoi(parts[2]),
                              parts[3]);
      else
         fprintf (stderr, "Incomplete line (obj-mark):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-msg-start") == 0)
      controller->objMsgStart (info, args);
   else if (strcmp (cmd, "obj-msg-end") == 0)
      controller->objMsgEnd (info, args);
   else if (strcmp (cmd, "obj-enter") == 0) {
      parts = split (args, 5);
      if (parts[1] && parts[2] && parts[3] && parts[4])
         controller->objEnter (info, parts[0], parts[1], atoi(parts[2]),
                               parts[3], parts[4]);
      else
         fprintf (stderr, "Incomplete line (obj-enter):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-leave") == 0)
      // Pre-version "obj-leave" does not support values.
      controller->objLeave (info, args, NULL);
   else if (strcmp (cmd, "obj-create") == 0) {
      parts = split (args, 2);
      if (parts[1])
         controller->objCreate (info, parts[0], parts[1]);
      else
         fprintf (stderr, "Incomplete line (obj-create):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-ident") == 0) {
      parts = split (args, 2);
      if (parts[1])
         controller->objIdent (info, parts[0], parts[1]);
      else
         fprintf (stderr, "Incomplete line (obj-ident):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-assoc") == 0) {
      parts = split (args, 2);
      if (parts[1])
         controller->objAssoc (info, parts[0], parts[1]);
      else
         fprintf (stderr, "Incomplete line (obj-assoc):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-set") == 0) {
      parts = split (args, 3);
      if (parts[1] && parts[2])
         controller->objSet (info, parts[0], parts[1], parts[2]);
      else
         fprintf (stderr, "Incomplete line (obj-set):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-color") == 0) {
      parts = split (args, 2);
      if (parts[1]) {
         fprintf (stderr, "Warning: obj-color is deprecated; use "
                  "obj-class-color instead:\n%s\n", info->completeLine);
         controller->objClassColor (info, parts[1], parts[0]);
      } else
         fprintf (stderr, "Incomplete line (obj-color):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-class-color") == 0) {
      parts = split (args, 2);
      if (parts[1])
         controller->objClassColor (info, parts[1], parts[0]);
      else
         fprintf (stderr, "Incomplete line (obj-class-color):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-object-color") == 0) {
      parts = split (args, 2);
      if (parts[1])
         controller->objObjectColor (info, parts[0], parts[1]);
      else
         fprintf (stderr, "Incomplete line (obj-object-color):\n%s\n",
                  info->completeLine);
   } else if (strcmp (cmd, "obj-delete") == 0)
      controller->objDelete (info, args);
   else
      fprintf (stderr, "Unknown command identifier '%s':\n%s\n", cmd,
               info->completeLine);

   if (parts)
      freeSplit (parts);
}

void ObjectsParser::processVCommand (CommonLineInfo *info, const char *module,
                                     int majorVersion, int minorVersion,
                                     const char *cmd, char **args)
{
   if (strcmp (module, "obj") == 0) {
      if (majorVersion > 1)
         fprintf (stderr, "Last supported version is 1.0:\n%s\n",
                  info->completeLine);
      if (args[0] == NULL) {
         if (strcmp (cmd, "noident") == 0)
            controller->objNoIdent (info);
         else
            // All other commands need arguments.
            fprintf (stderr, "Missing arguments:%s\n", info->completeLine);
      } else if (strcmp (cmd, "msg") == 0) {
         if (args[1] && args[2] && args[3])
            controller->objMsg (info, args[0], args[1], atoi(args[2]), args[3]);
         else
            fprintf (stderr, "Incomplete line (msg):\n%s\n",
                     info->completeLine);
      } else if (strcmp (cmd, "mark") == 0) {
         if (args[1] && args[2] && args[3])
            controller->objMark (info, args[0], args[1], atoi(args[2]),
                                 args[3]);
         else
            fprintf (stderr, "Incomplete line (mark):\n%s\n",
                     info->completeLine);
      } else if (strcmp (cmd, "msg-start") == 0)
         controller->objMsgStart (info, args[0]);
      else if (strcmp (cmd, "msg-end") == 0)
         controller->objMsgEnd (info, args[0]);
      else if (strcmp (cmd, "enter") == 0) {
         if (args[1] && args[2] && args[3] && args[4])
            controller->objEnter (info, args[0], args[1], atoi(args[2]),
                                  args[3], args[4]);
         else
            fprintf (stderr, "Incomplete line (enter):\n%s\n",
                     info->completeLine);
      } else if (strcmp (cmd, "leave") == 0)
         // Args[1] may be NULL.
         controller->objLeave (info, args[0], args[1]);
      else if (strcmp (cmd, "create") == 0) {
         if (args[1])
            controller->objCreate (info, args[0], args[1]);
         else
            fprintf (stderr, "Incomplete line (create):\n%s\n",
                     info->completeLine);
      } else if (strcmp (cmd, "ident") == 0) {
         if (args[1])
            controller->objIdent (info, args[0], args[1]);
         else
            fprintf (stderr, "Incomplete line (ident):\n%s\n",
                     info->completeLine);
      } else if (strcmp (cmd, "assoc") == 0) {
         if (args[1])
            controller->objAssoc (info, args[0], args[1]);
         else
            fprintf (stderr, "Incomplete line (assoc):\n%s\n",
                     info->completeLine);
      } else if (strcmp (cmd, "set") == 0) {
         if (args[1] && args[2])
            controller->objSet (info, args[0], args[1], args[2]);
         else
            fprintf (stderr, "Incomplete line (set):\n%s\n",
                     info->completeLine);
      } else if (strcmp (cmd, "class-color") == 0) {
         if (args[1])
            // Notice the changed order.
            controller->objClassColor (info, args[0], args[1]);
         else
            fprintf (stderr, "Incomplete line (class-color):\n%s\n",
                     info->completeLine);
      } else if (strcmp (cmd, "object-color") == 0) {
         if (args[1])
            controller->objObjectColor (info, args[0], args[1]);
         else
            fprintf (stderr, "Incomplete line (object-color):\n%s\n",
                     info->completeLine);
      } else if (strcmp (cmd, "delete") == 0)
         controller->objDelete (info, args[0]);
      else
         fprintf (stderr, "Unknown command identifier '%s':\n%s\n", cmd,
                  info->completeLine);
   }      
}

void ObjectsParser::setLinesSource (LinesSource *source)
{
   this->source = source;
}

void ObjectsParser::timeout (int type)
{
   PRINTF ("ObjectsParser::timeout (%d)", type);

   if (timeout::getCount (type) == 0)
      ; // ownTimeout (timeout::getActualType (type));
   else
      controller->timeout (timeout::decType (type));
}

void ObjectsParser::finish ()
{
   controller->finish ();
}

void ObjectsParser::setObjectsSink (ObjectsSink *sink)
{
   // This would be the controller passed in the constructor.
}

void ObjectsParser::addTimeout (double secs, int type)
{
   PRINTF ("ObjectsParser::addTimeout (%g, %d)", secs, type);

   if (source)
      source->addTimeout (secs, timeout::incType (type));
   else
      fprintf (stderr, "addTimeout (%g, %d): no source\n", secs, type);
}

void ObjectsParser::removeTimeout (int type)
{
   if (source)
      source->removeTimeout (timeout::incType (type));
   else
      fprintf (stderr, "removeTimeout (%d): no source\n", type);
}

} // namespace objects

} // namespace rtfl
