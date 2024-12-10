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

#include "parser.hh"

#include <string.h>
#include <ctype.h>

namespace rtfl {

namespace tools {

void Parser::setLinesSource (LinesSource *source)
{
}

void Parser::processLine (char *line)
{
   char *lineCopy = strdup (line);
   
   if (strncmp (lineCopy, "[rtfl]", 6) == 0) {
      // Pre-version: starts with "[rtfl]".

      char **parts = split (lineCopy + 6, 5);

      if (parts[1] && parts[2] && parts[3]) {
         // Notice that parts[4] (arguments) is allowed to be NULL here.
         CommonLineInfo info =
            { parts[0], atoi(parts[1]), atoi(parts[2]), line };
         processCommand (&info, parts[3], parts[4]);
      } else
         fprintf (stderr, "Incomplete line:\n%s\n", line);

      freeSplit (parts);
   } else if (strncmp (lineCopy, "[rtfl-", 6) == 0) {
      // Versioned: starts with "[rtfl-<module>-<major>.<minor>]".

      int i = 6;
      while (isalpha (lineCopy[i]))
         i++;

      if (lineCopy[i] != '-')
         fprintf (stderr, "Expected '-' after module:\n%s\n", line);
      else {
         char *module = new char[i - 6 + 1];
         memcpy (module, lineCopy + 6, (i - 6) * sizeof (char));
         module[i - 6] = 0;

         i++;
         if (!isdigit (lineCopy[i]))
            fprintf (stderr, "Missing major version:\n%s\n", line);
         else {
            int majorVersion = 0, minorVersion = 0;
            
            while (isdigit (lineCopy[i])) {
               majorVersion = 10 * majorVersion + (lineCopy[i] - '0');
               i++;
            }
            
            if (majorVersion == 0)
               fprintf (stderr, "Major version must be positive:\n%s\n", line);
            else if (lineCopy[i] != '.')
               fprintf (stderr, "Expected '.' after major version:\n%s\n",
                        line);
            else if (!isdigit (lineCopy[i + 1]))
               fprintf (stderr, "Missing minor version:\n%s\n", line);
            else {
               i++;
               while (isdigit (lineCopy[i])) {
                  minorVersion = 10 * minorVersion + (lineCopy[i] - '0');
                  i++;
               }
               
               if (lineCopy[i] != ']')
                  fprintf (stderr, "Expected ']' after minor version:\n%s\n",
                           line);
               else {
                  char **parts = splitEscaped (lineCopy + i + 1);
                  
                  if (parts[1] && parts[2] && parts[3]) {
                     // Notice that parts[4] (first argument) is allowed to be
                     // NULL here.
                     CommonLineInfo info = { parts[0], atoi(parts[1]),
                                             atoi(parts[2]), line };
                     processVCommand (&info, module, majorVersion, minorVersion,
                                      parts[3], parts + 4);
                  } else
                     fprintf (stderr, "Incomplete line:\n%s\n", line);
                  
                  freeSplitEscaped (parts);
               }
            }
         }

         delete[] module;
      }
   }

   free (lineCopy);
}

void Parser::finish ()
{
}

void Parser::timeout (int type)
{
}

char **Parser::splitEscaped (char *txt)
{
   int numParts;
   char **parts;

   scanSplit (txt, &numParts, NULL);
   parts = new char*[numParts + 1];
   scanSplit (txt, NULL, parts);
   parts[numParts] = NULL;

   for (int i = 0; i < numParts; i++)
      unquote (parts[i]);

   return parts;
}

void Parser::scanSplit (char *txt, int *numParts, char **parts)
{
   int iChar, iPart;
   bool quoted;

   if (numParts)
      *numParts = 1;

   if (parts)
      parts[0] = txt;

   for (iChar = 0, iPart = 1; txt[iChar]; iChar++) {
      if (txt[iChar] == '\\' && txt[iChar + 1]) {
         iChar++;
         quoted = true;
      } else
         quoted = false;

      if (!quoted && txt[iChar] == ':') {
         if (parts) {
            txt[iChar] = 0;
            parts[iPart] = txt + iChar + 1;
         }

         iPart++;

         if (numParts)
            (*numParts)++;
      }
   }
}

void Parser::unquote (char *txt)
{
   int i, j;
   for (i = 0, j = 0; txt[i]; i++, j++) {
      if (txt[i] == '\\' && txt[i + 1])
         i++;
      txt[j] = txt[i];
   }
   txt[j] = 0;
}

// Free result of splitEscaped().
void Parser::freeSplitEscaped (char **parts)
{
   delete[] parts;
}

// Split without escaping.
char **Parser::split (char *txt, int maxNum)
{
   // Only maxNum splits. If less parts are found, less parts are
   // returned, so the caller should check the result (first part is
   // always defined). Notice that the original text buffer (txt) is
   // destroyed, for speed.

   //printf ("===> split ('%s', %d)\n", txt, maxNum);

   char **parts = new char*[maxNum + 1];

   char *start = txt;
   int i = 0;
   while (i < maxNum) {
      char *end = start;
      while (*end != 0 && *end != ':') end++;
      int endOfTxt = *end == 0;

      //printf ("                    start '%s'\n", start);
      //printf ("                    end '%s' (%d character(s))\n",
      //        end, (int)(end - start));

      parts[i] = start;

      if (i < maxNum -1)
         *end = 0;

      //printf ("---> %d: '%s'\n", i, start);

      i++;
      if (endOfTxt)
         break;

      start = endOfTxt ? end : end + 1;
   }

   parts[i] = NULL;
   return parts;
}

// Free result of split().
void Parser::freeSplit (char **parts)
{
   delete[] parts;
}

} // namespace tools

} // namespace rtfl
