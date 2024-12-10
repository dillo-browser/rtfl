/*
 * RTFL
 *
 * Copyright 2013-2015 Sebastian Geerken <sgeerken@dillo.org>
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

#include <stdlib.h>
#include "../debug_rtfl.hh"

class A
{
private:
   int a;

public:
   inline A (int a) { DBG_OBJ_CREATE ("x::A"); this->a = a; }
   inline void behaveAsA () { DBG_OBJ_MSGF ("", 0, "behaveAsA: a = %d", a); }
};

class B
{
private:
   int b;

public:
   inline B (int b) { DBG_OBJ_CREATE ("y::B"); this->b = b; }
   inline void behaveAsB () { DBG_OBJ_MSGF ("", 0, "behaveAsB: b = %d", b); }
};

class C: public A, public B
{
private:
   int c;

public:
   inline C (int c): A (c / 3), B (c / 2) {
      DBG_OBJ_CREATE ("z::C");
      DBG_OBJ_BASECLASS (A);
      DBG_OBJ_BASECLASS (B);
      this->c = c;
   }
   inline void behaveAsC () { DBG_OBJ_MSGF ("", 0, "behaveAsC: c = %d", c); }
};


int main (int argc, char *argv[])
{
   DBG_OBJ_CLASS_COLOR ("x::A", "#c0ff80");
   DBG_OBJ_CLASS_COLOR ("y::B", "#c0c0ff");
   DBG_OBJ_CLASS_COLOR ("z::C", "#ffa0a0");

   C c (6);
   c.behaveAsA ();
   c.behaveAsB ();
   c.behaveAsC ();
}
