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

#include "objects_buffer.hh"

using namespace lout::object;
using namespace lout::container::typed;
using namespace rtfl::tools;

namespace rtfl {

namespace objects {

ObjectsBuffer::ObjectCommand::ObjectCommand (CommandType type,
                                             CommonLineInfo *info,
                                             const char *fmt, ...)
{
   this->type = type;
   this->info.fileName = strdup (info->fileName);
   this->info.lineNo = info->lineNo;
   this->info.processId = info->processId;
   this->info.completeLine = strdup (info->completeLine);

   numArgs = strlen (fmt);
   args = new Arg[numArgs];

   va_list vargs;
   va_start (vargs, fmt);

   char *s;
   for (int i = 0; fmt[i]; i++) {
      args[i].type = fmt[i];
      switch (fmt[i]) {
      case 'd':
         args[i].d = va_arg(vargs, int);
         break;

      case 's':
         s = va_arg (vargs, char*);
         args[i].s = s ? strdup (s) : NULL;
         break;
      }
   }
}

ObjectsBuffer::ObjectCommand::~ObjectCommand ()
{
   free (info.fileName);
   free (info.completeLine);

   for (int i = 0; i < numArgs; i++)
      if (args[i].type == 's' && args[i].s)
         free (args[i].s);

   delete[] args;
}

// ----------------------------------------------------------------------
   
ObjectsBuffer::ObjectsBuffer (ObjectsController *successor)
{
   this->successor = successor;
   successor->setObjectsSource (this);

   commandsQueue = new Vector<ObjectCommand> (1, true);
   queued = false;

   setObjectsSink (successor);
}

ObjectsBuffer::~ObjectsBuffer ()
{
   delete commandsQueue;
}

void ObjectsBuffer::objMsg (CommonLineInfo *info, const char *id,
                            const char *aspect, int prio, const char *message)
{
   process (new ObjectCommand (MSG, info, "ssds", id, aspect, prio, message));
}

void ObjectsBuffer::objMark (CommonLineInfo *info, const char *id,
                             const char *aspect, int prio, const char *message)
{
   process (new ObjectCommand (MARK, info, "ssds", id, aspect, prio, message));
}

void ObjectsBuffer::objMsgStart (CommonLineInfo *info, const char *id)
{
   process (new ObjectCommand (MSG_START, info, "s", id));
}

void ObjectsBuffer::objMsgEnd (CommonLineInfo *info, const char *id)
{
   process (new ObjectCommand (MSG_END, info, "s", id));
}

void ObjectsBuffer::objEnter (CommonLineInfo *info, const char *id,
                              const char *aspect, int prio, const char *funname,
                              const char *args)
{
   process (new ObjectCommand (ENTER, info, "ssdss", id, aspect, prio, funname,
                               args));
}

void ObjectsBuffer::objLeave (CommonLineInfo *info, const char *id,
                              const char *vals)
{
   process (new ObjectCommand (LEAVE, info, "ss", id, vals));
}

void ObjectsBuffer::objCreate (CommonLineInfo *info, const char *id,
                               const char *klass)
{
   process (new ObjectCommand (CREATE, info, "ss", id, klass));
}

void ObjectsBuffer::objIdent (CommonLineInfo *info, const char *id1,
                              const char *id2)
{
   process (new ObjectCommand (IDENT, info, "ss", id1, id2));
}

void ObjectsBuffer::objNoIdent (CommonLineInfo *info)
{
   process (new ObjectCommand (NOIDENT, info, ""));
}

void ObjectsBuffer::objAssoc (CommonLineInfo *info, const char *parent,
                              const char *child)
{
   process (new ObjectCommand (ASSOC, info, "ss", parent, child));
}

void ObjectsBuffer::objSet (CommonLineInfo *info, const char *id,
                            const char *var, const char *val)
{
   process (new ObjectCommand (SET, info, "sss", id, var, val));
}

void ObjectsBuffer::objClassColor (CommonLineInfo *info, const char *klass,
                                   const char *color)
{
   process (new ObjectCommand (CLASS_COLOR, info, "ss", klass, color));
}

void ObjectsBuffer::objObjectColor (CommonLineInfo *info, const char *id,
                                    const char *color)
{
   process (new ObjectCommand (OBJECT_COLOR, info, "ss", id, color));
}

void ObjectsBuffer::objDelete (CommonLineInfo *info, const char *id)
{
   process (new ObjectCommand (DELETE, info, "s", id));
}

void ObjectsBuffer::queue ()
{
   queued = true;
}

void ObjectsBuffer::pass ()
{
   for (int i = 0; i < commandsQueue->size (); i++)
      pass (commandsQueue->get (i));
   
   commandsQueue->clear ();
   queued = false;
}

void ObjectsBuffer::process (ObjectCommand *command)
{
   if (queued)
      queue (command);
   else {
      pass (command);
      delete command;
   }
}

void ObjectsBuffer::queue (ObjectCommand *command)
{
   commandsQueue->put (command);
}

void ObjectsBuffer::pass (ObjectCommand *command)
{
   CommonLineInfo info = { command->info.fileName, command->info.lineNo,
                           command->info.processId,
                           command->info.completeLine };
   ObjectCommand::Arg *a = command->args;

   switch (command->type) {
   case MSG:
      successor->objMsg (&info, a[0].s, a[1].s, a[2].d, a[3].s);
      break;

   case MARK:
      successor->objMark (&info, a[0].s, a[1].s, a[2].d, a[3].s);
      break;

   case MSG_START:
      successor->objMsgStart (&info, a[0].s);
      break;

   case MSG_END:
      successor->objMsgEnd (&info, a[0].s);
      break;

   case ENTER:
      successor->objEnter (&info, a[0].s, a[1].s, a[2].d, a[3].s, a[4].s);
      break;

   case LEAVE:
      successor->objLeave (&info, a[0].s, a[1].s);
      break;

   case CREATE:
      successor->objCreate (&info, a[0].s, a[1].s);
      break;

   case IDENT:
      successor->objIdent (&info, a[0].s, a[1].s);
      break;

   case NOIDENT:
      successor->objNoIdent (&info);
      break;

   case ASSOC:
      successor->objAssoc (&info, a[0].s, a[1].s);
      break;

   case SET:
      successor->objSet (&info, a[0].s, a[1].s, a[2].s);
      break;

   case CLASS_COLOR:
      successor->objClassColor (&info, a[0].s, a[1].s);
      break;

   case OBJECT_COLOR:
      successor->objObjectColor (&info, a[0].s, a[1].s);
      break;

   case DELETE:
      successor->objDelete (&info, a[0].s);
      break;
   }
}
  
} // namespace objects

} // namespace rtfl
