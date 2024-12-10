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

#include "objcount_controller.hh"

using namespace rtfl::tools;

namespace rtfl {

namespace objects {

void ObjCountController::objMsg (CommonLineInfo *info, const char *id,
                                 const char *aspect, int prio,
                                 const char *message)
{
   table->registerObject (id);
}

void ObjCountController::objMark (CommonLineInfo *info, const char *id,
                                  const char *aspect, int prio,
                                  const char *message)
{
   table->registerObject (id);
}

   void ObjCountController::objMsgStart (CommonLineInfo *info, const char *id)
{
   table->registerObject (id);
}

void ObjCountController::objMsgEnd (CommonLineInfo *info, const char *id)
{
   table->registerObject (id);
}

void ObjCountController::objEnter (CommonLineInfo *info, const char *id,
                                   const char *aspect, int prio,
                                   const char *funname, const char *args)
{
   table->registerObject (id);
}

void ObjCountController::objLeave (CommonLineInfo *info, const char *id,
                                   const char *vals)
{
   table->registerObject (id);
}

void ObjCountController::objCreate (CommonLineInfo *info, const char *id,
                                    const char *klass)
{
   table->createObject (id, klass);
}

void ObjCountController::objIdent (CommonLineInfo *info, const char *id1,
                                   const char *id2)
{
   // TODO Is this not done by ObjdentController?
   table->addIdentity (id1, id2);
}

void ObjCountController::objNoIdent (CommonLineInfo *info)
{
}

void ObjCountController::objAssoc (CommonLineInfo *info, const char *parent,
                                   const char *child)
{
   table->registerObject (parent);
   table->registerObject (child);
}

void ObjCountController::objSet (CommonLineInfo *info, const char *id,
                                 const char *var, const char *val)
{
   table->registerObject (id);
}

void ObjCountController::objClassColor (CommonLineInfo *info, const char *klass,
                                        const char *color)
{
   table->setClassColor (klass, color);
}

void ObjCountController::objObjectColor (CommonLineInfo *info, const char *id,
                                         const char *color)
{
   table->registerObject (id);
}
   
void ObjCountController::objDelete (CommonLineInfo *info, const char *id)
{
   table->deleteObject (id);
}

} // namespace objects

} // namespace rtfl
