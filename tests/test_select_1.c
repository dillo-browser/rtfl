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
   int eos = 0;
   while (!eos) {
      fd_set readfds;
      FD_ZERO (&readfds);
      FD_SET (0, &readfds);

      struct timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;

      if (select (1, &readfds, NULL, NULL, &tv) == -1)
         syserr ("select failed");

      if (FD_ISSET (0, &readfds)) {
         char buf[1024];
         int n = read (0, buf, 1024);
         if (n == -1)
            syserr ("select read");
         else if (n == 0)
            eos = 1;
         else
            if (write (1, buf, n) == -1)
               syserr ("write failed");
      } else
         puts ("---------- timeout? ----------");
   }

   return 0;
}
