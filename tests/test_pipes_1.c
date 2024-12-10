/*
 * RTFL
 *
 * Copyright 2014 Sebastian Geerken <sgeerken@dillo.org>
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

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>

static void syserr (const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);
   vfprintf (stderr, fmt, args);
   fprintf (stderr, ": %s\n", strerror (errno));
   exit (1);
}

int main (int argc, char *argv[])
{
   int parent2child[2];
   char buf[2048];
   ssize_t n;

   if (pipe (parent2child) == -1) syserr ("pipe failed");

   switch (fork ()) {
   case -1:
      syserr ("fork failed");
      break;
      
   case 0:
      if (close (parent2child[1]) == -1)
         syserr ("close(%d) failed", parent2child[0]);
      if (close (0) == -1) syserr ("close(0) failed");
      if (dup2 (parent2child[0], 0) == -1)
         syserr ("dup2(%d, 0) failed", parent2child[0]);
      if (close (parent2child[0]) == -1)
         syserr ("close(%d) failed", parent2child[0]);

      do {
         if ((n = read (0, buf, sizeof (buf))) == -1) syserr ("read failed");
         fprintf (stderr, "[child] %d bytes read\n", (int)n);
      } while (n > 0);
      break;

   default:
      if (close (parent2child[0]) == -1)
         syserr ("close(%d) failed", parent2child[0]);
      if (write (parent2child[1], "Hi!", 3) == -1) syserr ("write failed");
      if (close (parent2child[1]) == -1)
         syserr ("close(%d) failed", parent2child[0]);

      break;
   }
   
   return 0;
}
