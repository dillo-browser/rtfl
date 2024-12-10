/*
 * RTFL
 *
 * Copyright 2015 Sebastian Geerken <sgeerken@dillo.org>
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

#include "objects_parser.hh"
#include "objects_writer.hh"
#include "objdelete_controller.hh"
#include "objident_controller.hh"

#include <fcntl.h>

using namespace rtfl::tools;
using namespace rtfl::objects;

int main(int argc, char **argv)
{
   LinesSourceSequence source (true);
   int fd = open (".rtfl", O_RDONLY);
   if (fd != -1)
      source.add (new BlockingLinesSource (fd));
   source.add (new BlockingLinesSource (0));

   ObjectsWriter writer;
   ObjIdentController identController (&writer);
   ObjDeleteController deleteController (&identController);
   ObjectsParser parser (&deleteController);
   source.setup (&parser);

   return 0;
}
