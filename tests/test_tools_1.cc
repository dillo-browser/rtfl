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

#include "common/tools.hh"

using namespace lout::object;
using namespace lout::misc;
using namespace lout::container::untyped;
using namespace rtfl::tools;

class TestObject: public Object
{
private:
   int n;
   
public:
   TestObject (int n);
   ~TestObject ();

   void intoStringBuffer (StringBuffer *sb);
};

TestObject::TestObject (int n)
{
   this->n = n;
}

TestObject::~TestObject ()
{
   printf("(~TestObject: %d)\n", n);
}

void TestObject::intoStringBuffer (StringBuffer *sb)
{
   sb->append ("TestObject: ");
   sb->appendInt (n);
}

int main(int argc, char **argv)
{
   String *id[4];
   TestObject *testObject[4];
   EquivalenceRelation rel (true, true);

   for (int i = 0; i < 4; i++) {
      char idBuf[] = { 'i', (char)('1' + i), 0 };
      id[i] = new String(idBuf);
      testObject[i] = new TestObject (i);

      // New key since they are deleted by EquivalenceRelation.remove()
      // but later still needed for EquivalenceRelation.contains().
      rel.put (new String(id[i]->chars ()), testObject[i]);
   }

   for (int i = 0; i < 4; i++) {
      puts ("--------------------------------------------------");
      for (int j = 0; j < 4; j++) {
         if (rel.contains (id[j])) {
            Object *obj = rel.get (id[j]);
            StringBuffer sb;
            obj->intoStringBuffer (&sb);
            puts (sb.getChars ());
         } else
            puts ("(removed)");
      }

      if (i == 0)
         rel.relate (id[0], id[1]);

      if (i == 1)
         rel.relate (id[0], id[2]);

      if (i == 2)
         rel.remove (id[0]);
   }

   for (int i = 0; i < 4; i++)
      delete id[i];
   
   puts ("--------------------------------------------------");
}
