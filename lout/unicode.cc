/*
 * RTFL (originally part of dillo)
 *
 * Copyright 2012, 2013 Sebastian Geerken <sgeerken@dillo.org>
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


#include "unicode.hh"
#include "misc.hh"

using namespace lout::misc;

namespace lout {

namespace unicode {

static unsigned char alpha[0x500] = {
   // 0000-007F: C0 Controls and Basic Latin
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xfe, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x07,
   // 0080-00FF: C1 Controls and Latin-1 Supplement
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff,
   // 0100-017F: Latin Extended-A
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   // 0180-024F: Latin Extended-B
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff,
   // 0250–02AF: IPA Extensions
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff,
   // 02B0–02FF: Spacing Modifier Letters
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00,
   // 0300–036F: Combining Diacritical Marks
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   // 0370–03FF: Greek and Coptic
   0xcf, 0x00, 0x40, 0x7d, 0xff, 0xff, 0xfb, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff,
   // 0400–04FF: Cyrillic
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x03, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/**
 * Returns whether a given unicode character is an alphabetic character.
 */
bool isAlpha (int ch)
{
   return ch < 0x500 && (alpha[ch / 8] & (1 << (ch & 7)));
}

int decodeUtf8 (const char *s)
{
   if((s[0] & 0x80) == 0)
      return s[0];
   else if((s[0] & 0xe0) == 0xc0 && (s[1] & 0xc0) == 0x80)
      return ((s[0] & 0x1f) << 6) | (s[1] & 0x3f);
   else if((s[0] & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80
           && (s[2] & 0xc0) == 0x80)
      return ((s[0] & 0x0f) << 12) | ((s[1] & 0x3f) << 6)  | (s[2] & 0x3f);
   else if((s[0] & 0xf8) == 0xf0 && (s[1] & 0xc0) == 0x80
           && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80)
      return ((s[0] & 0x0f) << 18) | ((s[1] & 0x3f) << 12)
         | ((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
   else
      // Treat as ISO-8859-1 / ISO-8859-15 / Windows-1252
      return s[0];
}


int decodeUtf8 (const char *s, int len)
{
   if(len >= 1 && (s[0] & 0x80) == 0)
      return s[0];
   else if(len >= 2 && (s[0] & 0xe0) == 0xc0 && (s[1] & 0xc0) == 0x80)
      return ((s[0] & 0x1f) << 6) | (s[1] & 0x3f);
   else if(len >= 3 && (s[0] & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80
           && (s[2] & 0xc0) == 0x80)
      return ((s[0] & 0x0f) << 12) | ((s[1] & 0x3f) << 6)  | (s[2] & 0x3f);
   else if(len >= 4 && (s[0] & 0xf8) == 0xf0 && (s[1] & 0xc0) == 0x80
           && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80)
      return ((s[0] & 0x0f) << 18) | ((s[1] & 0x3f) << 12)
         | ((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
   else
      // Treat as ISO-8859-1 / ISO-8859-15 / Windows-1252
      return s[0];
}

const char *nextUtf8Char (const char *s)
{
   const char *r;

   if (s == NULL || s[0] == 0)
      r = NULL;
   else if((s[0] & 0x80) == 0)
      r = s + 1;
   else if((s[0] & 0xe0) == 0xc0 && (s[1] & 0xc0) == 0x80)
      r = s + 2;
   else if((s[0] & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80
           && (s[2] & 0xc0) == 0x80)
      r = s + 3;
   else if((s[0] & 0xf8) == 0xf0 && (s[1] & 0xc0) == 0x80
           && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80)
      r = s + 4;
   else
      // invalid UTF-8 sequence: treat as one byte.
      r = s + 1;

   if (r && r[0] == 0)
      return NULL;
   else
      return r;
}

const char *nextUtf8Char (const char *s, int len)
{
   const char *r;

   if (s == NULL || len <= 0)
      r = NULL;
   else if(len >= 1 && (s[0] & 0x80) == 0)
      r = s + 1;
   else if(len >= 2 && (s[0] & 0xe0) == 0xc0 && (s[1] & 0xc0) == 0x80)
      r = s + 2;
   else if(len >= 3 && (s[0] & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80
           && (s[2] & 0xc0) == 0x80)
      r = s + 3;
   else if(len >= 4 && (s[0] & 0xf8) == 0xf0 && (s[1] & 0xc0) == 0x80
           && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80)
      r = s + 4;
   else
      // invalid UTF-8 sequence: treat as one byte.
      r = s + 1;

   if (r && r - s >= len)
      return NULL;
   else
      return r;
}

int numUtf8Chars (const char *s)
{
   int numUtf8 = 0;
   for (const char *r = s; r; r = nextUtf8Char (r))
      numUtf8++;
   return numUtf8;
}

int numUtf8Chars (const char *s, int len)
{
   int numUtf8 = 0;
   for (const char *r = s; len > 0 && r; r = nextUtf8Char (r, len))
      numUtf8++;
   return numUtf8;
}

} // namespace lout

} // namespace unicode
