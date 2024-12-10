/*
 * RTFL
 *
 * Copyright 2015 Sebastian Geerken <sgeerken@dillo.org>
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

#include "objdelete_controller.hh"

using namespace lout::object;
using namespace lout::misc;
using namespace lout::container::typed;
using namespace rtfl::tools;

namespace rtfl {

namespace objects {

ObjDeleteController::ObjInfo::ObjInfo (const char *id)
{
   numCreated = 0;
   numDeleted = 0;
   origId = strdup (id);
   mappedId = strdup (id);
}

ObjDeleteController::ObjInfo::~ObjInfo ()
{
   free (origId);
   free (mappedId);
}

void ObjDeleteController::ObjInfo::use ()
{
   numCreated = max (numCreated, 1);
}

void ObjDeleteController::ObjInfo::objCreate ()
{
   numCreated++;
}

void ObjDeleteController::ObjInfo::objDelete ()
{
   numCreated--;
   if (numCreated <= 0) {
      numCreated = 0;
      numDeleted++;

      free (mappedId);
      mappedId = (char*) malloc ((strlen (origId) + 10 + 1) * sizeof (char));
      sprintf (mappedId, "%s-%d", origId, numDeleted);
   }
}

ObjDeleteController::ObjDeleteController (ObjectsController *successor)
{
   this->successor = successor;
   successor->setObjectsSource (this);
   setObjectsSink (successor);

   objInfos = new HashTable<String, ObjInfo> (true, true);
}

ObjDeleteController::~ObjDeleteController ()
{
   delete objInfos;
}

void ObjDeleteController::objMsg (CommonLineInfo *info, const char *id,
                                  const char *aspect, int prio,
                                  const char *message)
{
   successor->objMsg (info, mapId (id), aspect, prio, message);
}

void ObjDeleteController::objMark (CommonLineInfo *info, const char *id,
                                   const char *aspect, int prio,
                                   const char *message)
{
   successor->objMark (info, mapId (id), aspect, prio, message);
}

void ObjDeleteController::objMsgStart (CommonLineInfo *info, const char *id)
{
   successor->objMsgStart (info, mapId (id));
}

void ObjDeleteController::objMsgEnd (CommonLineInfo *info, const char *id)
{
   successor->objMsgEnd (info, mapId (id));
}

void ObjDeleteController::objEnter (CommonLineInfo *info, const char *id,
                                    const char *aspect, int prio,
                                    const char *funname, const char *args)
{
   successor->objEnter (info, mapId (id), aspect, prio, funname, args);
}

void ObjDeleteController::objLeave (CommonLineInfo *info, const char *id,
                                    const char *vals)
{
   successor->objLeave (info, mapId (id), vals);
}

void ObjDeleteController::objCreate (CommonLineInfo *info, const char *id,
                                     const char *klass)
{
   ensureObjInfo(id)->objCreate ();
   successor->objCreate (info, mapId (id), klass);
}

void ObjDeleteController::objIdent (CommonLineInfo *info, const char *id1,
                                    const char *id2)
{
   successor->objIdent (info, mapId (id1), mapId (id2));
}

void ObjDeleteController::objNoIdent (CommonLineInfo *info)
{
   successor->objNoIdent (info);
}

void ObjDeleteController::objAssoc (CommonLineInfo *info, const char *parent,
                                    const char *child)
{
   successor->objAssoc (info, mapId (parent), mapId (child));
}

void ObjDeleteController::objSet (CommonLineInfo *info, const char *id,
                                  const char *var, const char *val)
{
   successor->objSet (info, mapId (id), var, val);
}

void ObjDeleteController::objClassColor (CommonLineInfo *info,
                                         const char *klass, const char *color)
{
   successor->objClassColor (info, klass, color);
}

void ObjDeleteController::objObjectColor (CommonLineInfo *info, const char *id,
                                          const char *color)
{
   successor->objObjectColor (info, mapId (id), color);
}

void ObjDeleteController::objDelete (CommonLineInfo *info, const char *id)
{
   successor->objDelete (info, mapId (id));
   ensureObjInfo(id)->objDelete ();
}

const char *ObjDeleteController::mapId (const char *id)
{
   ObjInfo *objInfo = ensureObjInfo (id);
   objInfo->use ();
   return objInfo->getMappedId ();
}

ObjDeleteController::ObjInfo *ObjDeleteController::getObjInfo (const char *id)
{
   String key (id);
   return objInfos->get (&key);
}

ObjDeleteController::ObjInfo *ObjDeleteController::ensureObjInfo (const char
                                                                  *id)
{
   ObjInfo *objInfo = getObjInfo (id);
   if (objInfo == NULL) {
      objInfo = new ObjInfo (id);
      objInfos->put (new String (id), objInfo);
   }
   return objInfo;
}
 
} // namespace objects

} // namespace rtfl

