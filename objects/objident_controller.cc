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

#include <stdio.h>
#include "objident_controller.hh"

using namespace lout::object;
using namespace lout::misc;
using namespace lout::container::untyped;
using namespace rtfl::tools;

#define TIMEOUT_SECS 1

namespace rtfl {

namespace objects {

ObjIdentController::PostController::PostController (ObjectsController
                                                    *successor)
{
   this->successor = successor;
   successor->setObjectsSource (this);
   setObjectsSink (successor);

   // The "canonocal" identity is stored both as key and as value. For
   // this reason, the second argument could be "false". Check whether
   // this is possible (order of delete etc.).
   identities = new EquivalenceRelation (true, true);
}

ObjIdentController::PostController::~PostController ()
{
   delete identities;
}

void ObjIdentController::PostController::objMsg (CommonLineInfo *info,
                                                 const char *id,
                                                 const char *aspect, int prio,
                                                 const char *message)
{
   successor->objMsg (info, mapId (id), aspect, prio, message);
}

void ObjIdentController::PostController::objMark (CommonLineInfo *info,
                                                  const char *id,
                                                  const char *aspect, int prio,
                                                  const char *message)
{
   successor->objMark (info, mapId (id), aspect, prio, message);
}

void ObjIdentController::PostController::objMsgStart (CommonLineInfo *info,
                                                      const char *id)
{
   successor->objMsgStart (info, mapId (id));
}

void ObjIdentController::PostController::objMsgEnd (CommonLineInfo *info,
                                                    const char *id)
{
   successor->objMsgEnd (info, mapId (id));
}

void ObjIdentController::PostController::objEnter (CommonLineInfo *info,
                                                   const char *id,
                                                   const char *aspect, int prio,
                                                   const char *funname,
                                                   const char *args)
{
   successor->objEnter (info, mapId (id), aspect, prio, funname, args);
}

void ObjIdentController::PostController::objLeave (CommonLineInfo *info,
                                                   const char *id,
                                                   const char *vals)
{
   successor->objLeave (info, mapId (id), vals);
}

void ObjIdentController::PostController::objCreate (CommonLineInfo *info,
                                                    const char *id,
                                                    const char *klass)
{
   successor->objCreate (info, mapId (id), klass);
}

void ObjIdentController::PostController::objIdent (CommonLineInfo *info,
                                                   const char *id1,
                                                   const char *id2)
{
   // "obj-ident" is not delegated.
}

void ObjIdentController::PostController::objNoIdent (CommonLineInfo *info)
{
   // "obj-noident" is not delegated.
}

void ObjIdentController::PostController::objAssoc (CommonLineInfo *info,
                                                   const char *parent,
                                                   const char *child)
{
   successor->objAssoc (info, mapId (parent), mapId (child));
}

void ObjIdentController::PostController::objSet (CommonLineInfo *info,
                                                 const char *id,
                                                 const char *var,
                                                 const char *val)
{
   successor->objSet (info, mapId (id), var, val);
}

void ObjIdentController::PostController::objClassColor (CommonLineInfo *info,
                                                        const char *klass,
                                                        const char *color)
{
   successor->objClassColor (info, klass, color);
}

void ObjIdentController::PostController::objObjectColor (CommonLineInfo *info,
                                                         const char *id,
                                                         const char *color)
{
   successor->objObjectColor (info, mapId (id), color);
}

void ObjIdentController::PostController::objDelete (CommonLineInfo *info,
                                                    const char *id)
{
   successor->objDelete (info, mapId (id));

   // TODO Remove from identities.
}

void ObjIdentController::PostController::addIdentity (const char *id1,
                                                      const char *id2)
{
   String key1 (id1), key2 (id2);
   
   if (!identities->contains (&key1))
      identities->put (new String (id1), new String (id1));
   
   if (!identities->contains (&key2))
      identities->put (new String (id2), new String (id2));

   identities->relate (&key1, &key2);
}

const char *ObjIdentController::PostController::mapId (const char *id)
{
   String key (id);
   if (!identities->contains (&key))
       identities->put (new String (id), new String (id));

   return ((String*)identities->get(&key))->chars ();
}

// ----------------------------------------------------------------------

ObjIdentController::ObjIdentController (ObjectsController *successor)
{
   noIdent = false;
   stackDepth = 0;
   createPending = false;

   postController = new PostController (successor);
   buffer = new ObjectsBuffer (postController);
   buffer->setObjectsSource (this);
   setObjectsSink (buffer);
}

ObjIdentController::~ObjIdentController ()
{
}

void ObjIdentController::objMsg (CommonLineInfo *info, const char *id,
                                 const char *aspect, int prio,
                                 const char *message)
{
   buffer->objMsg (info, id, aspect, prio, message);
}

void ObjIdentController::objMark (CommonLineInfo *info, const char *id,
                                  const char *aspect, int prio,
                                  const char *message)
{
   buffer->objMark (info, id, aspect, prio, message);
}

void ObjIdentController::objMsgStart (CommonLineInfo *info, const char *id)
{
   buffer->objMsgStart (info, id);
}

void ObjIdentController::objMsgEnd (CommonLineInfo *info, const char *id)
{
   buffer->objMsgEnd (info, id);
}

void ObjIdentController::objEnter (CommonLineInfo *info, const char *id,
                                   const char *aspect, int prio,
                                   const char *funname, const char *args)
{
   stackDepth++;

   buffer->objEnter (info, id, aspect, prio, funname, args);
}

void ObjIdentController::objLeave (CommonLineInfo *info, const char *id,
                                   const char *vals)
{
   stackDepth = max (stackDepth - 1, 0);

   if (createPending && stackDepth < minCreateStackDepth) {
      pass ();
      removeOwnTimeout (PASS);
   }

   buffer->objLeave (info, id, vals);
}

void ObjIdentController::objCreate (CommonLineInfo *info, const char *id,
                                    const char *klass)
{
   queue ();
   buffer->objCreate (info, id, klass);
}

void ObjIdentController::objIdent (CommonLineInfo *info, const char *id1,
                                   const char *id2)
{
   postController->addIdentity (id1, id2);

   // TODO: Possibly end queueing? Probably not.
   
   // "obj-ident" is not delegated.
}

void ObjIdentController::objNoIdent (CommonLineInfo *info)
{
   noIdent = true;
   pass ();

   // "obj-noident" is not delegated.
}

void ObjIdentController::objAssoc (CommonLineInfo *info, const char *parent,
                                   const char *child)
{
   buffer->objAssoc (info, parent, child);
}

void ObjIdentController::objSet (CommonLineInfo *info, const char *id,
                                 const char *var, const char *val)
{
   buffer->objSet (info, id, var, val);
}

void ObjIdentController::objClassColor (CommonLineInfo *info, const char *klass,
                                        const char *color)
{
   buffer->objClassColor (info, klass, color);
}

void ObjIdentController::objObjectColor (CommonLineInfo *info, const char *id,
                                         const char *color)
{
   buffer->objObjectColor (info, id, color);
}

void ObjIdentController::objDelete (CommonLineInfo *info, const char *id)
{
   buffer->objDelete (info, id);
}

void ObjIdentController::ownTimeout (int type)
{
   if (type == PASS)
      pass ();
}      

void ObjIdentController::ownFinish ()
{
   pass ();
}

void ObjIdentController::queue ()
{
   if (!noIdent) {
      if (createPending)
         minCreateStackDepth = min (minCreateStackDepth, stackDepth);
      else {
         buffer->queue ();
         createPending = true;
         minCreateStackDepth = stackDepth;
      }

      removeOwnTimeout (PASS);
      addOwnTimeout (TIMEOUT_SECS, PASS);
   }
}      

void ObjIdentController::pass ()
{
   buffer->pass ();
   createPending = false;
}      
  
} // namespace objects

} // namespace rtfl

