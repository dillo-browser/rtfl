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
#include "objview_controller.hh"

using namespace rtfl::tools;

namespace rtfl {

namespace objects {

void ObjViewController::objMsg (CommonLineInfo *info, const char *id,
                                const char *aspect, int prio,
                                const char *message)
{
   graph->addCommand (new OVGAddMessageCommand (info->fileName, info->lineNo,
                                                graph, id, aspect, prio,
                                                message,
                                                graph->getLastEnterCommand ()),
                      true);
}

void ObjViewController::objMark (CommonLineInfo *info, const char *id,
                                 const char *aspect, int prio,
                                 const char *message)
{
   OVGAddMarkCommand *markCommand =
      new OVGAddMarkCommand (info->fileName, info->lineNo, graph, id, aspect,
                             prio, message, graph->getLastEnterCommand ());
   graph->addCommand (markCommand, true);
   graph->addCommandMark (markCommand);
}

void ObjViewController::objMsgStart (CommonLineInfo *info, const char *id)
{
   OVGIncIndentCommand *startCommand =
      new OVGIncIndentCommand (info->fileName, info->lineNo, graph, id,
                               graph->getLastEnterCommand ());
   graph->addCommand (startCommand, true);
   graph->pushStartCommand (startCommand);
}

void ObjViewController::objMsgEnd (CommonLineInfo *info, const char *id)
{
   OVGIncIndentCommand *lastStartCommand = graph->getLastStartCommand ();
   if (lastStartCommand == NULL)
      fprintf (stderr, "'obj-msg-end' without 'obj-msg-start:\n%s\n",
               info->completeLine);
   else {
      graph->addCommand (new OVGDecIndentCommand (info->fileName, info->lineNo,
                                                  graph, id, lastStartCommand,
                                                  graph->getLastEnterCommand()),
                         true);
      graph->popStartCommand ();
   }
}

void ObjViewController::objEnter (CommonLineInfo *info, const char *id,
                                  const char *aspect, int prio,
                                  const char *funname, const char *args)
{
   OVGEnterCommand *enterCommand =
      new OVGEnterCommand (info->fileName, info->lineNo, graph, id, aspect,
                           prio, funname, args, graph->getLastEnterCommand ());
   graph->addCommand (enterCommand, true);
   graph->pushEnterCommand (enterCommand);
}

void ObjViewController::objLeave (CommonLineInfo *info, const char *id,
                                  const char *vals)
{
   OVGEnterCommand *lastEnterCommand = graph->getLastEnterCommand ();
   if (lastEnterCommand == NULL)
      fprintf (stderr, "'obj-leave' without 'obj-enter:\n%s\n",
               info->completeLine);
   else {
      graph->addCommand (new OVGLeaveCommand (info->fileName, info->lineNo,
                                              graph, id, vals,
                                              lastEnterCommand),
                         true);
      graph->popEnterCommand ();
   }
}

void ObjViewController::objCreate (CommonLineInfo *info, const char *id,
                                   const char *klass)
{
   graph->addCommand (new OVGCreateCommand (info->fileName, info->lineNo, graph,
                                            id, klass,
                                            graph->getLastEnterCommand ()),
                      true);
}

void ObjViewController::objIdent (CommonLineInfo *info, const char *id1,
                                  const char *id2)
{
   // TODO Is this not done by ObjdentController?
   graph->addIdentity (id1, id2);
}

void ObjViewController::objNoIdent (CommonLineInfo *info)
{
}

void ObjViewController::objAssoc (CommonLineInfo *info, const char *parent,
                                  const char *child)
{
   graph->addCommand (new OVGAddAssocCommand (info->fileName, info->lineNo,
                                              graph, parent, child,
                                              graph->getLastEnterCommand ()),
                      true);
}

void ObjViewController::objSet (CommonLineInfo *info, const char *id,
                                const char *var, const char *val)
{
   graph->addCommand (new OVGAddAttrCommand (info->fileName, info->lineNo,
                                             graph, id, var, val,
                                             graph->getLastEnterCommand ()),
                      true);
}

void ObjViewController::objClassColor (CommonLineInfo *info, const char *klass,
                                       const char *color)
{
   graph->setClassColor (klass, color);
}

void ObjViewController::objObjectColor (CommonLineInfo *info, const char *id,
                                        const char *color)
{
   graph->setObjectColor (id, color);
}

void ObjViewController::objDelete (CommonLineInfo *info, const char *id)
{
   graph->addCommand (new OVGDeleteCommand (info->fileName, info->lineNo, graph,
                                            id, graph->getLastEnterCommand ()),
                      true);
}

} // namespace objects

} // namespace rtfl
