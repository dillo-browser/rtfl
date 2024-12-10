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

// See "collect-stats" in this directory.

#include "debug_rtfl.hh"
#include "lout/object.hh"
#include "lout/container.hh"

using namespace lout::object;
using namespace lout::container::typed;


static void sortvec (Vector<Integer> *vec)
{
   DBG_GEN_TIME ();
   DBG_OBJ_ENTER0_O ("", 0, NULL, "sortvec");

   DBG_OBJ_MSGF_O ("", 0, NULL, "size = %d", vec->size ());
   
   vec->sort ();

   DBG_GEN_TIME ();
   DBG_OBJ_LEAVE_O (NULL);
}

int main (int argc, char *argv[])
{
   for (int i = 0; i < 20; i++) {
      int n = 20000 * i;
      Vector<Integer> vec (n, true);
      for (int j = 0; j < n; j++)
         vec.put (new Integer (random ()));
      sortvec (&vec);
   }
   
   return 0;
}
