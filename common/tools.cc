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

#include "tools.hh"

#include <stdio.h>
#include <errno.h>

using namespace lout::object;
using namespace lout::container::untyped;

namespace rtfl {

namespace tools {

const char *numSuffix (int n)
{
   if (n % 10 == 1 && n != 11)
      return "st";
   else if (n % 10 == 2 && n != 12)
      return "nd";
   else if (n % 10 == 3 && n != 13)
      return "rd";
   else
      return "th";
}

static const char
   *const roman_I0[] = { "","I","II","III","IV","V","VI","VII","VIII","IX" },
   *const roman_I1[] = { "","X","XX","XXX","XL","L","LX","LXX","LXXX","XC" },
   *const roman_I2[] = { "","C","CC","CCC","CD","D","DC","DCC","DCCC","CM" },
   *const roman_I3[] = { "","M","MM","MMM","MMMM" };

void numToRoman (int num, char *buf, int buflen)
{
   int i3, i2, i1, i0;

   if (buflen <= 0)
      return;

   i0 = num;
   i1 = i0/10; i2 = i1/10; i3 = i2/10;
   i0 %= 10;   i1 %= 10;   i2 %= 10;
   if (num < 0 || i3 > 4) /* more than 4999 elements ? */
      snprintf(buf, buflen, "****");
   else
      snprintf(buf, buflen, "%s%s%s%s", roman_I3[i3], roman_I2[i2],
               roman_I1[i1], roman_I0[i0]);
}

void syserr (const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);
   vfprintf (stderr, fmt, args);
   fprintf (stderr, ": %s\n", strerror (errno));
   exit (1);
}

// ----------------------------------------------------------------------

EquivalenceRelation::RefTarget::RefTarget (Object *object, bool ownerOfObject)
{
   this->object = object;
   this->ownerOfObject = ownerOfObject;
   refCount = 1;
   allKeys = new HashSet (false);
}

EquivalenceRelation::RefTarget::~RefTarget ()
{
   if (ownerOfObject)
      delete object;
   delete allKeys;
}

// ----------------------------------------------------------------------

EquivalenceRelation::RefSource::RefSource (Object *key, RefTarget *target)
{
   this->target = target;
   this->key = key;
   refTarget ();
}

EquivalenceRelation::RefSource::~RefSource ()
{
   unrefTarget ();
}

void EquivalenceRelation::RefSource::refTarget ()
{
   if (target) {
      target->ref ();
      target->putKey (key);
   }
}

void EquivalenceRelation::RefSource::unrefTarget ()
{
   if (target) {
      target->removeKey (key);
      target->unref ();
      target = NULL;
   }
}
   
void EquivalenceRelation::RefSource::setTarget (RefTarget *target)
{
   if (target != this->target) {
      unrefTarget ();
      this->target = target;
      refTarget ();
   }
}

// ----------------------------------------------------------------------

EquivalenceRelation::EquivalenceRelation (bool ownerOfKeys, bool ownerOfValues)
{
   this->ownerOfKeys = ownerOfKeys;
   this->ownerOfValues = ownerOfValues;
   sources = new HashTable (ownerOfKeys, true);
}

EquivalenceRelation::~EquivalenceRelation ()
{
   delete sources;
}

void EquivalenceRelation::put (Object *key, Object *value)
{
   assert (!contains(key));

   RefTarget *target = new RefTarget (value, ownerOfValues);
   RefSource *source = new RefSource (key, target);
   target->unref ();
   sources->put (key, source);
}

Object *EquivalenceRelation::get (Object *key) const
{
   RefSource *source = (RefSource*) sources->get(key);
   if (source) {
      Object *object = source->getTarget()->getObject ();
      return object;
   } else
      return NULL;
}

bool EquivalenceRelation::contains (Object *key) const
{
   return sources->contains (key);
}

Iterator EquivalenceRelation::iterator ()
{
   return sources->iterator ();
}

Iterator EquivalenceRelation::relatedIterator (Object *key)
{
   assert (contains (key));

   RefSource *source = (RefSource*) sources->get (key);
   RefTarget *target = source->getTarget ();
   return target->getAllKeys()->iterator ();
}

void EquivalenceRelation::relate (Object *key1, Object *key2)
{
   assert (contains(key1) && contains(key2));

   RefSource *source1 = (RefSource*) sources->get (key1);
   RefSource *source2 = (RefSource*) sources->get (key2);
   if (source1->getTarget () != source2->getTarget ()) {
      // The first value is kept, the second destroyed. The caller has
      // to care about the order.

      // Consider all keys already related to `key2`; this is possible by
      // iterating over `RefTarget::allKeys`. To avoid accessing freed memory,
      // copy all keys to a new temporary set.
      
      HashSet target2Keys (false);
      for (Iterator it = source2->getTarget()->getAllKeys()->iterator ();
           it.hasNext (); )
         target2Keys.put (it.getNext ());
      
      for (Iterator it = target2Keys.iterator (); it.hasNext (); ) {
         RefSource *otherSource = (RefSource*) sources->get (it.getNext ());
         otherSource->setTarget (source1->getTarget ());
      }
   }
}

void EquivalenceRelation::putRelated (Object *oldKey, Object *newKey)
{
   assert (contains(oldKey) && !contains(newKey));

   RefSource *oldSource = (RefSource*) sources->get (oldKey);
   RefSource *newSource = new RefSource (newKey, oldSource->getTarget ());
   sources->put (newKey, newSource);
}
   
void EquivalenceRelation::removeSimple (lout::object::Object *key)
{
   // The order is important: a simple "sources->remove (key)" will
   // cause an access to freed memory.
      
   RefSource *source = (RefSource*) sources->get (key);
   source->setTarget (NULL); // Will unref() the target.
   sources->remove (key); 
}
   
void EquivalenceRelation::remove (Object *key)
{
   assert (contains (key));

   RefSource *source = (RefSource*) sources->get (key);
   RefTarget *target = source->getTarget ();
   target->ref ();

   for (Iterator it = target->getAllKeys()->iterator (); it.hasNext (); ) {
      Object *otherKey = it.getNext ();

      // The order is important: see removeSimple().
      RefSource *otherSource = (RefSource*) sources->get (otherKey);
      otherSource->setTarget (NULL); // Will unref() the target.
      sources->remove (otherKey);
   }

   target->unref ();
}


} // namespace tools

} // namespace dw
