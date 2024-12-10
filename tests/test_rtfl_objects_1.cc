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
#include "debug_rtfl.hh"
#include "lout/object.hh"
#include "lout/container.hh"

using namespace lout::object;
using namespace lout::container::typed;

class A
{
private:
   A *other;
   List<Integer> *numbers;
   
public:
   A ();
   ~A ();
   void setOther (A *other);
   int doSomething (int n);
};

class B: public A
{
public:
   B ();
};

class C: public A
{
public:
   C ();
};


A::A ()
{
   DBG_OBJ_CREATE ("A");

   other = NULL;
   numbers = new List<Integer> (true);
}

A::~A ()
{
   delete numbers;
}

void A::setOther (A *other)
{
   DBG_OBJ_ASSOC_CHILD (other);

   this->other = other;
}

int A::doSomething (int n)
{
   DBG_OBJ_ENTER ("", 0, "doSomething", "%d", n);

   DBG_OBJ_MSGF ("", 0, "some message: n = %d", n);

   int r = random () % 251;
   numbers->append (new Integer (r));
   DBG_OBJ_ARRSET_NUM ("numbers", numbers->size () - 1, r);
      
   if (other && n > 0)
      other->doSomething (n - 1);

   DBG_OBJ_LEAVE_VAL ("%d", r);
   return r;
}

B::B ()
{
   DBG_OBJ_CREATE ("B");
}

C::C ()
{
   DBG_OBJ_CREATE ("C");
}

int main (int argc, char *argv[])
{
   DBG_OBJ_CLASS_COLOR ("A", "#ffa0a0");
   DBG_OBJ_CLASS_COLOR ("B", "#60ff60");
   DBG_OBJ_CLASS_COLOR ("C", "#b0b0ff");

   A x;
   B y;
   C z;

   x.setOther (&y);
   y.setOther (&z);
   z.setOther (&x);

   x.doSomething (8);

   return 0;
}
