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

#include "objects_writer.hh"

#define DBG_RTFL

#include "debug_rtfl.hh"

using namespace rtfl::tools;

// If foreign messages ("F" stands for "foreign") are passed, the original
// filename, line number and process id must be printed.

#define F_RTFL_PRINT(module, version, cmd, fmt, ...) \
   rtfl_print (module, version, info->fileName, info->lineNo, info->processId, \
               "s:" fmt, cmd, __VA_ARGS__)

#define F_RTFL_PRINT0(module, version, cmd) \
   rtfl_print (module, version, info->fileName, info->lineNo, info->processId, \
               "s", cmd)

#define F_RTFL_OBJ_PRINT(cmd, fmt, ...) \
   F_RTFL_PRINT ("obj", RTFL_OBJ_VERSION, cmd, fmt, __VA_ARGS__)

#define F_RTFL_OBJ_PRINT0(cmd) \
   F_RTFL_PRINT0 ("obj", RTFL_OBJ_VERSION, cmd)

namespace rtfl {

namespace objects {

void ObjectsWriter::objMsg (CommonLineInfo *info, const char *id,
                            const char *aspect, int prio, const char *message)
{
   F_RTFL_OBJ_PRINT ("msg", "s:s:d:s", id, aspect, prio, message);
}

void ObjectsWriter::objMark (CommonLineInfo *info, const char *id,
                             const char *aspect, int prio, const char *message)
{
   F_RTFL_OBJ_PRINT ("mark", "s:s:d:s", id, aspect, prio, message);
}

void ObjectsWriter::objMsgStart (CommonLineInfo *info, const char *id)
{
   F_RTFL_OBJ_PRINT ("msg-start", "s", id);
}

void ObjectsWriter::objMsgEnd (CommonLineInfo *info, const char *id)
{
   F_RTFL_OBJ_PRINT ("msg-end", "s", id);
}

void ObjectsWriter::objEnter (CommonLineInfo *info, const char *id,
                              const char *aspect, int prio, const char *funname,
                              const char *args)
{
   F_RTFL_OBJ_PRINT ("enter", "s:s:d:s:s", id, aspect, prio, funname, args);
}

void ObjectsWriter::objLeave (CommonLineInfo *info, const char *id,
                              const char *vals)
{
   if (vals)
      F_RTFL_OBJ_PRINT ("leave", "s:s", id, vals);
   else
      F_RTFL_OBJ_PRINT ("leave", "s", id);
}

void ObjectsWriter::objCreate (CommonLineInfo *info, const char *id,
                               const char *klass)
{
   F_RTFL_OBJ_PRINT ("create", "s:s", id, klass);
}

void ObjectsWriter::objIdent (CommonLineInfo *info, const char *id1,
                              const char *id2)
{
   F_RTFL_OBJ_PRINT ("create", "s:s", id1, id2);
}

void ObjectsWriter::objNoIdent (CommonLineInfo *info)
{
   F_RTFL_OBJ_PRINT0 ("noident");
}

void ObjectsWriter::objAssoc (CommonLineInfo *info, const char *parent,
                              const char *child)
{
   F_RTFL_OBJ_PRINT ("assoc", "s:s", parent, child);
}
   
void ObjectsWriter::objSet (CommonLineInfo *info, const char *id,
                            const char *var, const char *val)
{
   F_RTFL_OBJ_PRINT ("set", "s:s:s", id, var, val);
}

void ObjectsWriter::objClassColor (CommonLineInfo *info, const char *klass,
                                   const char *color)
{
   F_RTFL_OBJ_PRINT ("class-color", "s:s", klass, color);
}

void ObjectsWriter::objObjectColor (CommonLineInfo *info, const char *id,
                                    const char *color)
{
   F_RTFL_OBJ_PRINT ("object-color", "s:s", id, color);
}

void ObjectsWriter::objDelete (CommonLineInfo *info, const char *id)
{
   F_RTFL_OBJ_PRINT ("delete", "s", id);
}

} // namespace objects

} // namespace rtfl
