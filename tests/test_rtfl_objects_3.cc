/*
 * RTFL
 *
 * Copyright 2014, 2015 Sebastian Geerken <sgeerken@dillo.org>
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
   A *otherA;
   
public:
   A ();
   ~A ();
   void setOtherA (A *newOtherA);
   void doSomething (int level);
   void doActualStuff ();
};

A::A ()
{
   DBG_OBJ_CREATE ("A");
   otherA = NULL;
}

A::~A ()
{
   DBG_OBJ_DELETE ();
}

void A::setOtherA (A *newOtherA)
{
   DBG_OBJ_ENTER ("all", 0, "setOtherA", "%p", newOtherA);

   otherA = newOtherA;
   DBG_OBJ_ASSOC_CHILD (otherA);

   DBG_OBJ_LEAVE ();
}

void A::doSomething (int level)
{
   DBG_OBJ_ENTER ("all", 0, "doSomething", "%d", level);

   if (level > 0) {
      doActualStuff ();
      if (otherA)
         otherA->doSomething (level - 1);
   }

   DBG_OBJ_LEAVE ();
}

void A::doActualStuff ()
{
   DBG_OBJ_ENTER0 ("all", 0, "doActualStuff");

   DBG_OBJ_MSG ("all", 1, "(pretending ...)");
  
   DBG_OBJ_LEAVE ();
}

int main (int argc, char *argv[])
{
   A a1, a2;
   a1.setOtherA (&a2);
   a2.setOtherA (&a1);
   a1.doSomething (20);
}
