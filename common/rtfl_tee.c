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
 * Like tee(1), this program duplicates a stream; however, it does not
 * write the copy to a file, but instead sends it via pipe to another
 * program. Example:
 *
 * $ foo | rtfl-tee bar | qix
 *
 * Here, the standard output of "foo" is passed to the standard input
 * of both "bar" and "qix".
 *
 * More informations in doc/rtfl.html.
 *
 * TODO: Something like "echo -n foo | rtfl-tee -b cat" does not work;
 * since the line of the first "foo" is never finished, the copy of
 * "foo" is never printed.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

static void usrerr (const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);
   vfprintf (stderr, fmt, args);
   fprintf (stderr, "\n");
   exit (1);
}

static void syserr (const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);
   vfprintf (stderr, fmt, args);
   fprintf (stderr, ": %s\n", strerror (errno));
   exit (1);
}

static ssize_t ewrite(int fd, const void *buf, size_t count)
{
   ssize_t w;
   if ((w = write (fd, buf, count)) == -1)
      syserr ("write(%d, ...) failed", fd);
   return w;
}

static void writestdout (int orig, char *buf, size_t count)
{
   // Basic idea: "orig" denotes to 0 (stdin of rtfl-tee) or 1 (stdout
   // of the called program). "curorig" refers to the origin which is
   // currently printed, so than the data from the other origin must
   // be buffered. "startline" is set to 1 at the beginning, or iff
   // the last printed character was '\n'. (In this case, switching is
   // simply possible.)

   static int curorig = 0, startline = 1;
   static char obuf[2048];
   static size_t ocount = 0;

   //printf ("\nwritestdout: %d, '%c...' (%d)\n",
   //        orig, count > 0 ? buf[0] : '.', (int)count);
   //printf ("===> curorig = %d, ocount = %d, startline = %d\n",
   //        curorig, (int)ocount, startline);
   
   if (count > 0) {
      if (orig != curorig) {
         if (startline) {
            // Simple switching case.
            ewrite (1, obuf, ocount);
            ocount = 0;
            curorig = orig;
            ewrite (1, buf, count);
            startline = buf[count - 1] == '\n';
         } else {
            // Buffer.
            size_t odiff = min (count, ocount - sizeof (obuf));
            memcpy (obuf + ocount, buf, odiff);
            ocount += odiff;
         }
      } else {
         if (ocount == 0) {
            // Nothing buffered: simply print all data.
            ewrite (1, buf, count);
            startline = buf[count - 1] == '\n';
         } else {
            // Only print everything until the last newline character.
            // (Note: printing everything until the *first* newline
            // character whould make a larger buffer necessary, but,
            // on the other hand, preserve better the original
            // (temporal) order of the lines.)
            ssize_t i, nl;
            for (nl = -1, i = count - 1; nl == -1 && i >= 0; i--)
               if (buf[i] == '\n') nl = i;
         
            if (nl == -1) {
               // No newline: no switch.
               ewrite (1, buf, count);
               startline = 0;
            } else {
               // Newline: switch.
               ewrite (1, buf, nl + 1);
               ewrite (1, obuf, ocount);
               startline = obuf[ocount - 1] == '\n';
               ocount = min (sizeof (obuf), count - (nl + 1));
               memcpy (obuf, buf + nl + 1, ocount);
               curorig = 1 - curorig;
            }
         }
      }
   }
}

int main (int argc, char *argv[])
{
   int parent2child[2], child2parent[2], i, offsetcmd, bypass = 0, erroropt = 0;
   char *argv2[argc - 1 + 1];
   int done1, done2;

   for (offsetcmd = 1; offsetcmd < argc && argv[offsetcmd][0] == '-';
        offsetcmd++) {
      if (argv[offsetcmd][1] == 'b')
         bypass = 1;
      else if (argv[offsetcmd][1] == '-') {
         offsetcmd++;
         break;
      } else
         erroropt = 1;                  
   }

   if (erroropt || offsetcmd >= argc)
      usrerr ("Usage: %s [-b] [--] <program> [<program options>]", argv[0]);

   if (pipe (parent2child) == -1) syserr ("pipe failed");
   if (bypass && pipe (child2parent) == -1) syserr ("pipe failed");

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

      if (bypass) {
         if (close (child2parent[0]) == -1)
            syserr ("close(%d) failed", child2parent[1]);
         if (close (1) == -1) syserr ("close(1) failed");
         if (dup2 (child2parent[1], 1) == -1)
            syserr ("dup2(%d, 1) failed", child2parent[1]);
         if (close (child2parent[1]) == -1)
            syserr ("close(%d) failed", child2parent[1]);
      }
      
      for (i = 0; i < argc - offsetcmd; i++) argv2[i] = argv[i + offsetcmd];
      argv2[argc - offsetcmd] = NULL;
      if (execvp (argv2[0], argv2) == -1)
         syserr ("execvp(\"%s\", ...) failed", argv2[0]);
      break;
      
   default:
      if (close (parent2child[0]) == -1)
         syserr ("close(%d) failed", parent2child[0]);
      if (bypass && close (child2parent[1]) == -1)
         syserr ("close(%d) failed", child2parent[1]);

      done1 = 0;
      done2 = !bypass;
      while (!done1 || !done2) {
         //printf ("==> done1 = %d, done2 = %d\n", done1, done2);

         fd_set set;
         FD_ZERO (&set);
            
         if (!done1) FD_SET (0, &set);
         if (!done2) FD_SET (child2parent[0], &set);

         int s = select ((!done2 ? child2parent[0] : 0) + 1,
                         &set, NULL, NULL, NULL);

         //printf ("==> s = %d\n", s);

         if (s == -1) syserr ("select failed");
         else if (s > 0) {
            char buf[2048];
            ssize_t n;

            if (!done1 && FD_ISSET(0, &set)) {
               if ((n = read (0, buf, sizeof (buf))) == -1)
                  syserr ("read failed");
               else if (n == 0) {
                  if (close (parent2child[1]) == -1)
                     syserr ("close(%d) failed", parent2child[1]);
                  done1 = 1;
               } else if (n > 0) {
                  ewrite (parent2child[1], buf, n);
                  writestdout (0, buf, n);
               }
            } 
               
            if (!done2 && FD_ISSET (child2parent[0], &set)) {
               if ((n = read (child2parent[0], buf, sizeof (buf))) == -1)
                  syserr ("read failed");
               else if (n == 0) {
                  if (close (child2parent[0]) == -1)
                     syserr ("close(%d) failed", child2parent[0]);
                  done2 = 1;
               } else if (n > 0)
                  writestdout (1, buf, n);
            }
         }
      }
      break;
   }
   
   return 0;
}
