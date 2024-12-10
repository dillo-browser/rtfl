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

/*
 * Simply copy stdin to stdout, but slowly character by character.
 * Used to test rtfl-tee.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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
   char buf[2048];
   ssize_t n, i;

   do {
      if ((n = read (0, buf, sizeof (buf))) == -1) syserr ("read failed");
      for (i = 0; i < n; i++) {
         if (write (1, buf + i, 1) == -1) syserr ("write failed");
         usleep (250000);
      }
   } while (n > 0);

   return 0;
}
