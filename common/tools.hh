#ifndef __COMMON_TOOLS_HH__
#define __COMMON_TOOLS_HH__

#include "lout/object.hh"
#include "lout/container.hh"

namespace rtfl {

namespace tools {

const char *numSuffix (int n);
void numToRoman (int num, char *buf, int buflen);
void syserr (const char *fmt, ...);

class EquivalenceRelation: public lout::object::Object {
private:
   class RefTarget: public lout::object::Object {
   private:
      bool ownerOfObject;
      int refCount;
      lout::object::Object *object;
      lout::container::untyped::HashSet *allKeys;

   public:
      RefTarget (lout::object::Object *object, bool ownerOfObject);
      ~RefTarget ();

      inline lout::object::Object *getObject () { return object; }
      inline void ref () { refCount++; }
      inline void unref () { if (--refCount == 0) delete this; }

      inline lout::container::untyped::HashSet *getAllKeys ()
      { return allKeys; }
      inline void putKey (Object *key) { allKeys->put (key); }
      inline void removeKey (Object *key) { allKeys->remove (key); }
   };


   class RefSource: public lout::object::Object {
      RefTarget *target;
      lout::object::Object *key;

      void refTarget ();
      void unrefTarget ();
      
   public:
      RefSource (lout::object::Object *key, RefTarget *target);
      ~RefSource ();

      inline RefTarget *getTarget () { return target; }
      void setTarget (RefTarget *target);
   };
   
   bool ownerOfKeys, ownerOfValues;
   lout::container::untyped::HashTable *sources;

   lout::container::untyped::HashSet *initSet (lout::object::Object *o);

 public:
   EquivalenceRelation (bool ownerOfKeys, bool ownerOfValues);
   ~EquivalenceRelation ();
  
   void put (lout::object::Object *key, lout::object::Object *value);
   lout::object::Object *get (lout::object::Object *key) const;
   bool contains (lout::object::Object *key) const;
   lout::container::untyped::Iterator iterator ();
   lout::container::untyped::Iterator relatedIterator (Object *key);

   void relate (lout::object::Object *key1, lout::object::Object *key2);
   void putRelated (lout::object::Object *oldKey, lout::object::Object *newKey);
   
   void removeSimple (lout::object::Object *key);
   void remove (lout::object::Object *key);
};

} // namespace tools

} // namespace rtfl

#endif // __COMMON_TOOLS_HH__
