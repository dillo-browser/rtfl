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

#include "lines.hh"
#include "tools.hh"

#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/timeb.h>

#if 0
#   define PRINT(fmt) printf ("---- [%p] " fmt "\n", this)
#   define PRINTF(fmt, ...) printf ("---- [%p] " fmt "\n", this, __VA_ARGS__)
#else
#   define PRINT(fmt)
#   define PRINTF(fmt, ...)
#endif

using namespace lout::container::typed;
using namespace lout::misc;

namespace rtfl {

namespace tools {

// -----------------------------
//      LinesSourceSequence
// -----------------------------
   
LinesSourceSequence::VirtualSink::VirtualSink ()
{
}
   
void LinesSourceSequence::VirtualSink::processLine (char *line)
{
   sequence->sink->processLine (line);
}

void LinesSourceSequence::VirtualSink::setLinesSource (LinesSource *source)
{
}

void LinesSourceSequence::VirtualSink::finish ()
{
   // If a child source calls sink->finish() within setup(), this is
   // called recursively, but this does not cause problems.

   if (sequence->iterator.hasNext ()) {
      LinesSource *source = sequence->iterator.getNext ();
      source->setup (this);
   } else {
      sequence->sink->finish ();
   }
}

void LinesSourceSequence::VirtualSink::timeout (int type)
{
   sequence->sink->timeout (type);
}
   
LinesSourceSequence::LinesSourceSequence (bool ownerOfSources)
{
   virtualSink.sequence = this;
   sources = new List<LinesSource> (ownerOfSources);
   setupCalled = false;
}

LinesSourceSequence::~LinesSourceSequence ()
{
   delete sources;
}

void LinesSourceSequence::add (LinesSource *source)
{
   assert (!setupCalled);
   sources->append (source);
}

void LinesSourceSequence::setup (LinesSink *sink)
{
   this->sink = sink;
   sink->setLinesSource (this);
   setupCalled = true;
   iterator = sources->iterator ();
   virtualSink.finish ();
}

void LinesSourceSequence::addTimeout (double secs, int type)
{
   // TODO: After calling this, no source should be added.
   // TODO: Processed timeouts must be removed from other sources as well?
   
   // Sent to all, even if only one child source will actually trigger the
   // timeout; but we do not know which one.

   // In the real world, LinesSourceSequence is used for ".rtfl" and stdin, so
   // we do not have to worry too much about correctly handling timeouts.
   
   for (Iterator<LinesSource> it = sources->iterator (); it.hasNext (); ) {
      it.getNext()->addTimeout (secs, type);
   }
}

void LinesSourceSequence::removeTimeout (int type)
{
   for (Iterator<LinesSource> it = sources->iterator (); it.hasNext (); ) {
      it.getNext()->removeTimeout (type);
   }
}

// -------------------------
//      FileLinesSource
// -------------------------

FileLinesSource::FileLinesSource ()
{
   bufPos = 0;
   completeLine = true;
}

int FileLinesSource::processInput (int fd)
{
   int n;
   if ((n = read (fd, buf + bufPos, MAX_LINE_SIZE - bufPos)) > 0) {
      int bytesAvail = bufPos + n;
      int startOfLine = 0;
      bool lineProcessed;

      //printf ("--> %d bytes read, %d available\n", n, bytesAvail);

      do {
         lineProcessed = false;
         for (int i = startOfLine; !lineProcessed && i < bytesAvail; i++) {
            if (buf[i] == '\n') {
               buf[i] = 0;
              
               // If lines are too long (see below, where completeLine
               // is set to false), they are not processed.
               if (completeLine)
                  sink->processLine (buf + startOfLine);

               lineProcessed = true;
               startOfLine = i + 1;

               completeLine = true;
            }
         }
      } while (lineProcessed);

      memmove (buf, buf + startOfLine, bytesAvail - startOfLine);
      bufPos = bytesAvail - startOfLine;

      PRINTF ("processInput: %d bytes left in buffer", bufPos);
      
      // Handle case when line is to large (> MAX_LINE_SIZE
      // bytes). The whole line is discarded (completeLine), so we
      // empty the buffer by setting bufPos to 0.
      if (bufPos == MAX_LINE_SIZE) {
         bufPos = 0;
         completeLine = false;
      }

      //printf ("   --> %d processed, new pos: %d; will read %d\n",
      //        startOfLine, bufPos, MAX_LINE_SIZE - bufPos);
   } 

   //printf ("   --> read(2) returns %d\n", n);

   return n;
}
   
// -----------------------------
//      BlockingLinesSource
// -----------------------------

BlockingLinesSource::TimeoutInfo::TimeoutInfo (long time, int type)
{
   this->time = time;
   this->type = type;
}

bool BlockingLinesSource::TimeoutInfo::equals(Object *other)
{
   return time == ((TimeoutInfo*)other)->time &&
      type == ((TimeoutInfo*)other)->type;
}

int BlockingLinesSource::TimeoutInfo::hashValue()
{
   // This should better be hidden in lout::objects. Cf. Pointer::hashValue().
#if SIZEOF_LONG == 4
   return (int)time ^ type;
#else
   return ((intptr_t)time >> 32) ^ ((intptr_t)time) ^ type;
#endif
}

BlockingLinesSource::BlockingLinesSource (int fd)
{
   this->fd = fd;
   timeoutInfos = new HashSet<TimeoutInfo> (true);
}

BlockingLinesSource::~BlockingLinesSource ()
{
   delete timeoutInfos;
}
   
void BlockingLinesSource::setup (LinesSink *sink)
{
   setSink (sink);

   // We read non-blocking so that select(2) will work properly.
   // (FileLinesSource::processInput would block otherwise.)   
   int flags = fcntl(0, F_GETFL, 0);
   fcntl(0, F_SETFL, flags | O_NONBLOCK);

   bool eos = false;
   while (!eos) {
      fd_set readfds;
      FD_ZERO (&readfds);
      FD_SET (fd, &readfds);

      TimeoutInfo *nextTimeout = getNextTimeoutInfo ();

      struct timeval tv, *tvp;
      if (nextTimeout == NULL) {
         tvp = NULL;
         PRINT ("no timeout");
      } else {
         long tdelta = max (nextTimeout->getTime () - getCurrentTime (), 0L);
         tv.tv_sec = tdelta / 1000;
         tv.tv_usec = (tdelta % 1000) * 1000;
         tvp = &tv;
         PRINTF ("waiting %ld (%ld, %ld)", tdelta, tv.tv_sec, tv.tv_usec);
      }

      PRINT (">> processTimeouts");
      processTimeouts ();
      PRINT ("<< processTimeouts");

      PRINT (">> select");
      if (select (fd + 1, &readfds, NULL, NULL, tvp) == -1)
         syserr ("select failed");
      PRINT ("<< select");

      processTimeouts ();

      if (FD_ISSET (fd, &readfds)) {
         PRINT (">> processInput");
         int n = processInput (fd);
         PRINT ("<< processInput");
         if (n == 0) {
            eos = true;
         }
      }
   }

   close (fd);
   sink->finish ();
}

void BlockingLinesSource::addTimeout (double secs, int type)
{
   PRINTF ("addTimeout (%g, %d)", secs, type);
   timeoutInfos->put (new TimeoutInfo (getCurrentTime () + secs * 1000, type));
}

void BlockingLinesSource::removeTimeout (int type)
{
   PRINTF ("removeTimeout (%d)", type);

   // Iterators will not work when the set is modified; hence this nested loop.
   bool found;
   do {
      found = false;
      for (Iterator<TimeoutInfo> it = timeoutInfos->iterator ();
           !found && it.hasNext (); ) {
         TimeoutInfo *timeout = it.getNext();
         if (timeout->getType () == type) {
            found = true;
            timeoutInfos->remove (timeout);
         }
      }
   } while (found);
}

long BlockingLinesSource::getCurrentTime ()
{
   struct timeb t;
   if (ftime (&t) == -1)
      syserr ("ftime() failed");
   return t.time * 1000L + t.millitm;
}

BlockingLinesSource::TimeoutInfo *BlockingLinesSource::getNextTimeoutInfo ()
{
   TimeoutInfo *nextTimeout = NULL;
   
   for (Iterator<TimeoutInfo> it = timeoutInfos->iterator ();
        it.hasNext (); ) {
      TimeoutInfo *timeout = it.getNext();
      if (nextTimeout == NULL ||
          timeout->getTime () < nextTimeout->getTime ())
         nextTimeout = timeout;
   }

   return nextTimeout;
}

void BlockingLinesSource::processTimeouts ()
{
   long currentTime = getCurrentTime ();

   while (true) {
      TimeoutInfo *nextTimeout = getNextTimeoutInfo ();
      if (nextTimeout == NULL)
         break;

      PRINTF ("processTimeouts: %ld > %ld? %s",
              nextTimeout->getTime (), currentTime,
              nextTimeout->getTime () > currentTime ? "yes" : "no");
      if (nextTimeout->getTime () > currentTime)
         break;

      PRINT ("processTimeouts: call timeout");

      getSink()->timeout (nextTimeout->getType ());
      timeoutInfos->remove (nextTimeout);
   }
}

} // namespace tools

} // namespace rtfl
