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

#include "fltk_lines.hh"

#include <stdio.h>
#include <fcntl.h>
#include <Fl/Fl.H>

using namespace lout::container::typed;

namespace rtfl {

namespace common {

// -------------------------
//      FltkLinesSource
// -------------------------

FltkLinesSource::TimeoutInfo::TimeoutInfo (FltkLinesSource *source, int type)
{
   this->source = source;
   this->type = type;
}

FltkLinesSource::FltkLinesSource ()
{
   timeoutInfos = new List<TimeoutInfo> (true);
}

FltkLinesSource::~FltkLinesSource ()
{
   delete timeoutInfos;
}
   
void FltkLinesSource::staticProcessInputCallback (int fd, void *data)
{
   ((FltkLinesSource*)data)->processInputCallback (fd);
}

void FltkLinesSource::processInputCallback (int fd)
{
   int n = processInput (fd);

   if (n == 0) {
      // We read non-blocking, so -1 is returned and (errno set to
      // EAGAIN) when no data is currently available. When 0 is
      // returned, this means that there is permanently no data
      // (typically that the tested program has terminated). For some
      // reasons, the cpu is hogged then; this is avoided by removing
      // the read function again.
      Fl::remove_fd(0, FL_READ);
      getSink()->finish ();
   }
}

void FltkLinesSource::setup (tools::LinesSink *sink)
{
   setSink (sink);
   
   int flags = fcntl(0, F_GETFL, 0);
   fcntl(0, F_SETFL, flags | O_NONBLOCK);

   Fl::add_fd(0, FL_READ, staticProcessInputCallback, (void*)this);
}

void FltkLinesSource::addTimeout (double secs, int type)
{
   TimeoutInfo *timeoutInfo = new TimeoutInfo (this, type);
   timeoutInfos->append (timeoutInfo);
   Fl::add_timeout(secs, timeoutCallback, timeoutInfo);
}

void FltkLinesSource::timeoutCallback (void *data)
{
   TimeoutInfo *timeoutInfo = (TimeoutInfo*) data;
   timeoutInfo->getSource()->getSink()->timeout (timeoutInfo->getType ());
   timeoutInfo->getSource()->timeoutInfos->removeRef (timeoutInfo);
}

void FltkLinesSource::removeTimeout (int type)
{      
   // Iterators will not work when the set is modified; hence this nested loop.
   bool found;
   do {
      found = false;
      for (Iterator<TimeoutInfo> it = timeoutInfos->iterator ();
           !found && it.hasNext (); ) {
         TimeoutInfo *timeout = it.getNext();
         if (timeout->getType () == type) {
            found = true;
            Fl::remove_timeout(timeoutCallback, timeout);
            timeoutInfos->removeRef (timeout);
         }
      }
   } while (found);
}

// ---------------------------
//      FltkDefaultSource
// ---------------------------
   
FltkDefaultSource::FltkDefaultSource (): LinesSourceSequence (true)
{
   int fd = open (".rtfl", O_RDONLY);
   if (fd != -1)
      add (new tools::BlockingLinesSource (fd));

   add (new FltkLinesSource ());
}

} // namespace objects

} // namespace rtfl
