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

/*
 * This program searches for identical sequences in a stream of RTFL
 * messages (actually, any stream) and marks the beginnings and ends
 * with an RTFL mark (obj-mark). You should first filter out other
 * lines, so run
 *
 * $ ... | grep '^\[rtfl[^\]]*\]' | rtfl-findrepeat
 *
 * or use the script rtfl-objfilter.
 *
 * For options, see printHelp().
 *
 * Warning: This program is highly experimental, and especially rather
 * inefficient. Some ideas:
 *
 * - When finding a suitable lenght ("-l find"), a smaller number of
 *   lines is in many cases sufficient, so use "head".
 *
 * - Unless the length is searched for ("-l find"), hashing multiple
 *   lines (as many as are searched as minimum), as done in the
 *   searching algorithm by Rabin and Karp, may increase the speed.
 */

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "tools.hh"
#include "../lout/object.hh"
#include "../lout/container.hh"

using namespace lout::misc;
using namespace lout::object;
using namespace lout::container::typed;

enum { MAX_LINE_SIZE = 1000 };

class Region: public Comparable
{
   int first, num;

public:
   inline Region (int first, int num) { this->first = first; this->num = num; }

   bool equals (Object *other);
   int hashValue ();
   void intoStringBuffer (StringBuffer *sb);
   int compareTo(Comparable *other);

   inline int getFirst () { return first; }
   inline int getNum () { return num; }
   inline Region *cloneRegion () { return new Region (first, num); }

   inline bool subSetOf (Region *other)
   { return first >= other->first && first + num <= other->first + other->num; }
};

class Mark: public Object
{
public:
   enum Type { START, END };

private:
   Type type;
   int majorNo, minorNo, length;

public:
   inline Mark (Type type, int majorNo, int minorNo, int length)
   { this->type = type; this->majorNo = majorNo; this->minorNo = minorNo;
      this->length = length; }

   void intoStringBuffer (StringBuffer *sb);

   inline Type getType () { return type; }
   inline int getMajorNo () { return majorNo; }
   inline int getMinorNo () { return minorNo; }
   inline int getLength () { return length; }
};

bool Region::equals (Object *other)
{
   Region *otherRegion = (Region*)other;
   return first == otherRegion->first && num == otherRegion->num;
}

int Region::hashValue ()
{
   return first ^ num;
}

void Region::intoStringBuffer (StringBuffer *sb)
{
   char buf[32];

   sb->append ("(");
   snprintf (buf, 32, "%d", first);
   sb->append (buf);
   sb->append ("...");
   snprintf (buf, 32, "%d", first + num - 1);
   sb->append (buf);
   sb->append (")");
}

void Mark::intoStringBuffer (StringBuffer *sb)
{
   char buf[32];


   sb->append ("(");
   sb->append (type == START  ? "START" : "END");
   sb->append (" / ");
   snprintf (buf, 32, "%d", majorNo);
   sb->append (buf);
   sb->append (" / ");
   snprintf (buf, 32, "%d", minorNo);
   sb->append (buf);
   sb->append (")");
}

int Region::compareTo(Comparable *other)
{
   Region *otherRegion = (Region*)other;
   return first - otherRegion->first;
}

// ----------------------------------------------------------------------

static bool debug = false;

static void printHelp (const char *argv0)
{
   fprintf
      (stderr, "Usage: %s <options>\n"
       "\n"
       "Options:\n"
       "   -l <n>           Search for sequence of at least <n> lines.\n"
       "   -c <n>           Search for sequence repeated at least <n> times.\n"
       "\n"
       "If an arguments is 'f' or 'find', the maximal value for this is\n"
       "determined (possibly with the other argument set to a concrete\n"
       "number).\n"
       "\n"
       "See RTFL documentation for more details.\n",
       argv0);  
}

// ----------------------------------------------------------------------

static void readFile (FILE *file, Vector<String> *lines,
                      HashTable<String, Vector<Integer> > *lineNosByLines)
{
      char buf[MAX_LINE_SIZE + 1];

      for (int lineNo = 0; fgets (buf, MAX_LINE_SIZE + 1, file); lineNo++) {
      size_t l = strlen (buf);
      if (buf[l - 1] == '\n') buf[l - 1] = 0;

      String *line = new String (buf);

      Vector<Integer> *lineNos = lineNosByLines->get (line);
      if (lineNos == NULL) {
         lineNos = new Vector<Integer> (1, true);
         // Note: key is dublicated.
         lineNosByLines->put (new String (buf), lineNos);
      }
      lineNos->put (new Integer (lineNo));

      lines->put (line);
   }
}

static int findRegions (Vector<String> *lines,
                        HashTable<String, Vector<Integer> > *lineNosByLines,
                        List <HashSet<Region> > *allSetsOfRegions,
                        int minLength, int minCount)
{
   int effMinLength = minLength == -1 ? 2 :minLength;
   int effMinCount = minCount == -1 ? 2 : minCount;
   int maxLength = 0, maxCount = 0;
   
   HashTable<Region, HashSet<Region> > *setsOfRegionsByRegion =
      new HashTable<Region, HashSet<Region> > (false, false);

   List <HashSet<Region> > *tmpAllSetsOfRegions =
      new List<HashSet<Region> > (false);

   for (int lineNo1 = 0; lineNo1 < lines->size (); lineNo1++) {
      String *line = lines->get (lineNo1);
      Vector<Integer> *lineNos = lineNosByLines->get (line);

      // Examine only lines after this.
      Integer lineNo1Key (lineNo1);
      
      for (int linesNoIndex = lineNos->bsearch (&lineNo1Key, true) + 1; 
           linesNoIndex < lineNos->size (); linesNoIndex++) {
         int lineNo2 = lineNos->get(linesNoIndex)->getValue ();
         int numMatching = 1;
         while (lineNo2 + numMatching < lines->size () &&
                lines->get(lineNo1 + numMatching)->equals 
                   (lines->get(lineNo2 + numMatching))) {
            numMatching++;

            if (numMatching >= effMinLength) {
               //printf ("equal: (%d...%d) and (%d...%d)\n",
               //        lineNo1, lineNo1 + numMatching - 1,
               //        lineNo2, lineNo2 + numMatching - 1);

               Region r1 (lineNo1, numMatching), r2 (lineNo2, numMatching);
               HashSet<Region> *setOfRegions;

               if ((setOfRegions = setsOfRegionsByRegion->get (&r1))) {
                  if (!setsOfRegionsByRegion->contains (&r2)) {
                     assert (!setOfRegions->contains (&r2));
                     Region *rr2 = r2.cloneRegion ();
                     setOfRegions->put (rr2);
                     setsOfRegionsByRegion->put (rr2, setOfRegions);
                  }
               } else if ((setOfRegions = setsOfRegionsByRegion->get (&r2))) {
                  if (!setsOfRegionsByRegion->contains (&r1)) {
                     assert (!setOfRegions->contains (&r1));
                     Region *rr1 = r1.cloneRegion ();
                     setOfRegions->put (rr1);
                     setsOfRegionsByRegion->put (rr1, setOfRegions);
                  }
               } else {
                  Region *rr1 = r1.cloneRegion (), *rr2 = r2.cloneRegion ();
                  setOfRegions = new HashSet<Region> (false);
                  setOfRegions->put (rr1);
                  setOfRegions->put (rr2);
                  setsOfRegionsByRegion->put (rr1, setOfRegions);
                  setsOfRegionsByRegion->put (rr2, setOfRegions);
                  tmpAllSetsOfRegions->append (setOfRegions);
               }

               if (debug) {
                  StringBuffer sb;
                  setsOfRegionsByRegion->intoStringBuffer (&sb);
                  printf ("findRegions: setsOfRegionsByRegion = %s\n",
                          sb.getChars ());
               }
            }
         }
      }
   }

   delete setsOfRegionsByRegion;

   for (Iterator<HashSet<Region> > it1 = tmpAllSetsOfRegions->iterator ();
        it1.hasNext (); ) {
      HashSet<Region> *set = it1.getNext ();
      if (set->size () >= effMinCount) {
         allSetsOfRegions->append (set);
         maxCount = max (maxCount, set->size ());
         
         if (minLength == -1) {
            for (Iterator<Region> it2 = set->iterator (); it2.hasNext (); ) {
               Region *r = it2.getNext ();
               maxLength = max (maxLength, r->getNum ());
            }
         }
      } else
         delete set;
   }

   delete tmpAllSetsOfRegions;

   if (minLength == -1)
      return maxLength;
   else if (minCount == -1)
      return maxCount;
   else
      return -1;
}

static void sortListsOfRegions (List <HashSet<Region> > *allSetsOfRegions,
                                List <Vector<Region> > *allListsOfRegions)
{
   for (Iterator<HashSet<Region> > it1 = allSetsOfRegions->iterator ();
        it1.hasNext (); ) {
      HashSet<Region> *set = it1.getNext ();
      Vector<Region> *list = new Vector<Region> (1, true);
      
      for (Iterator<Region> it2 = set->iterator (); it2.hasNext (); ) {
         Region *r = it2.getNext ();
         list->put (r);
      }
         
      if (debug) {
         StringBuffer sb;
         list->intoStringBuffer (&sb);
         printf ("sortListsOfRegions: list = %s\n", sb.getChars ());
      }
      
      list->sort ();
      allListsOfRegions->append (list);
   }
}

static void cleanupRegions (List <Vector <Region> > *allListsOfRegions)
{
   HashTable<List<Integer>, Vector<Vector<Region> > > *allListsOfLists =
      new HashTable<List<Integer>, Vector<Vector<Region> > > (true, true);

   for (Iterator<Vector <Region> > it = allListsOfRegions->iterator ();
        it.hasNext (); ) {
      Vector<Region> *list = it.getNext ();

      List<Integer> *key = new List<Integer> (true);
      for (int i = 1; i < list->size (); i++)
         key->append (new Integer (list->get(i)->getFirst () -
                                   list->get(i - 1)->getFirst ()));
      
      Vector<Vector<Region> > *listOfLists = allListsOfLists->get (key);
      if (listOfLists)
         delete key;
      else {
         listOfLists = new Vector<Vector<Region> > (1, false);
         allListsOfLists->put (key, listOfLists);
      }

      listOfLists->put (list);
   }

   allListsOfRegions->clear ();

   if (debug) {
      StringBuffer sb;
      allListsOfLists->intoStringBuffer (&sb);
      printf ("cleanupRegions: allListsOfLists = %s\n", sb.getChars ());
   }

   for (Iterator<List<Integer> > it = allListsOfLists->iterator ();
        it.hasNext (); ) {
      List<Integer> *key = it.getNext ();
      Vector<Vector<Region> > *listOfLists = allListsOfLists->get (key);
      
      if (debug) {
         StringBuffer sb;
         listOfLists->intoStringBuffer (&sb);
         printf ("cleanupRegions: listOfLists = %s\n", sb.getChars ());
      }
      
      for (int i = 0; i < listOfLists->size (); i++) {
         Vector<Region> *list1 = listOfLists->get (i);
         Region *r1 = list1->get (0);
         bool redundant = false;
         for (int j = 0; j < listOfLists->size () && !redundant; j++) {
            if (i != j) {
               Vector<Region> *list2 = listOfLists->get (j);
               if (list2 != NULL) {
                  Region *r2 = list2->get (0);
                  if (r1->subSetOf (r2))
                     redundant = true;
               }
            }
         }

         if (redundant) {
            listOfLists->put (NULL, i);
            delete list1;
         } else
            allListsOfRegions->append (list1);
      }
   }

   delete allListsOfLists;
}

// ----------------------------------------------------------------------

int main (int argc, char *argv[])
{
   int minLength = 2, minCount = 2;
   int opt;

   while ((opt = getopt(argc, argv, "c:dl:")) != -1) {
      switch (opt) {
      case 'c':
         if (strcmp (optarg, "f") == 0 || strcmp (optarg, "find") == 0)
            minCount = -1;
         else
            minCount = atoi (optarg);
         break;

      case 'd':
         debug = true;
         break;

      case 'l':
         if (strcmp (optarg, "f") == 0 || strcmp (optarg, "find") == 0)
            minLength = -1;
         else
            minLength = atoi (optarg);
         break;

      default:
         printHelp (argv[0]);
         return 1;
      }
   }

   Vector<String> *lines = new Vector<String> (8, true);
   HashTable<String, Vector<Integer> > *lineNosByLines =
      new HashTable<String, Vector<Integer> > (true, true);

   readFile (stdin, lines, lineNosByLines);

   List <HashSet<Region> > *allSetsOfRegions =
      new List<HashSet<Region> > (true);

   int numFound = findRegions (lines, lineNosByLines, allSetsOfRegions,
                               minLength, minCount);

   if (debug) {
      StringBuffer sb;
      allSetsOfRegions->intoStringBuffer (&sb);
      printf ("main: allSetsOfRegions = %s\n", sb.getChars ());
   }

   delete lineNosByLines;

   if (numFound != -1) {
      delete allSetsOfRegions;
      printf ("%d\n", numFound);
   } else {
      List <Vector<Region> > *allListsOfRegions =
         new List <Vector<Region> > (false); // TODO Memory leak!

      sortListsOfRegions (allSetsOfRegions, allListsOfRegions);
      
      delete allSetsOfRegions;
      
      if (debug) {
         StringBuffer sb;
         allListsOfRegions->intoStringBuffer (&sb);
         printf ("(a) main: allListsOfRegions = %s\n", sb.getChars ());
      }

      cleanupRegions (allListsOfRegions);
      
      if (debug) {
         StringBuffer sb;
         allListsOfRegions->intoStringBuffer (&sb);
         printf ("(b) main: allListsOfRegions = %s\n", sb.getChars ());
      }
      
      HashTable<Integer, List<Mark> > *marksByLineNo =
         new HashTable<Integer, List<Mark> > (true, true);
      
      int majorNo = 0;
      for (Iterator<Vector<Region> > it1 = allListsOfRegions->iterator ();
           it1.hasNext (); ) {
         Vector<Region> *list = it1.getNext ();
         int minorNo = 0;
         
         for (Iterator<Region> it2 = list->iterator (); it2.hasNext (); ) {
            Region *r = it2.getNext ();
            
            for (int typeNo = 0; typeNo < 2; typeNo++) {
               Mark::Type type = typeNo == 0 ? Mark::START : Mark::END;
               int lineNo =
                  r->getFirst () + (type == Mark::START ? 0 : r->getNum ());
               Integer lineNoKey (lineNo);
               
               List<Mark> *list = marksByLineNo->get (&lineNoKey);
               if (list == NULL) {
                  list = new List<Mark> (true);
                  marksByLineNo->put (new Integer (lineNo), list);
               }
               
               list->append (new Mark (type, majorNo, minorNo, r->getNum ()));
            }
            
            
            minorNo++;
         }
         
         majorNo++;
      }
      
      delete allListsOfRegions;
      
      if (debug) {
         StringBuffer sb;
         marksByLineNo->intoStringBuffer (&sb);
         printf ("main: marksByLineNo = %s\n", sb.getChars ());
      }

      for (int lineNo = 0; lineNo < lines->size (); lineNo++) {
         Integer lineNoKey (lineNo);
         List<Mark> *list = marksByLineNo->get (&lineNoKey);
         if (list) {
            for (Iterator<Mark> it = list->iterator (); it.hasNext (); ) {
               Mark *m = it.getNext ();
               char buf[200];
               rtfl::tools::numToRoman (m->getMajorNo () + 1, buf,
                                        sizeof (buf));
               // Certainly no ':' or '\' in the message, so no quoting
               // necessary.
               printf ("[rtfl-obj-1.0]n:0:0:mark:findrepeat:findrepeat:0:"
                       "Sequence %s (length %d), %d%s occurence -- %s\n",
                       buf, m->getLength (), m->getMinorNo () + 1,
                       rtfl::tools::numSuffix (m->getMinorNo () + 1),
                       m->getType () == Mark::START ? "start" : "end");
            }
         }

         String *line = lines->get (lineNo);
         puts (line->chars ());
      }

      delete marksByLineNo;
   }

   delete lines;
}
