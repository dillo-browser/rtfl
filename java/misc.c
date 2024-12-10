#include <string.h>
#include <stdlib.h>

#include "misc.h"

bool str_starts_with (const char *haystack, const char *needle)
{
   int haystack_len = strlen (haystack), needle_len = strlen (needle);
   if (haystack_len < needle_len)
      return FALSE;
   else
      return memcmp (haystack, needle, needle_len * sizeof (char)) == 0;
}

void jvmti_error (jvmtiEnv *jvmti, jvmtiError error, const char *fmt, ...)
{
   char *errorname;
   if ((*jvmti)->GetErrorName (jvmti, error, &errorname) != JVMTI_ERROR_NONE)
      errorname = "<error getting error name>";
   
   fprintf(stderr, "error (jvmtiError %d: %s): ", error, errorname);

   va_list argp;
   va_start(argp, fmt);
   vfprintf(stderr, fmt, argp);
   va_end(argp);
   
   fprintf(stderr, "\n");
}

void other_error (const char *fmt, ...)
{
   fprintf(stderr, "error (other): ");

   va_list argp;
   va_start(argp, fmt);
   vfprintf(stderr, fmt, argp);
   va_end(argp);
   
   fprintf(stderr, "\n");
}

void jvmti_dealloc (jvmtiEnv *jvmti, void *mem)
{
   if (mem) {
      jvmtiError error;
      if ((error = (*jvmti)->Deallocate (jvmti, mem)) != JVMTI_ERROR_NONE)
         jvmti_error (jvmti, error, "Deallocate");
   }
}

void simple_free (void *mem)
{
   if (mem)
      free (mem);
}

char *get_class_name_from_sig (const char *class_sig, bool expect_class)
{
   int len_class_sig = strlen (class_sig);
   if (!(class_sig[0] == 'L' && class_sig[len_class_sig - 1] == ';')) {
      if (expect_class)
         other_error ("don't know how to deal with class signature '%s'",
                      class_sig);
      return NULL;
   } else {
      char *class_name =
         (char*)malloc ((len_class_sig - 2 + 1) * sizeof (char));
      int i;
      for (i = 0; i < len_class_sig - 2; i++)
         class_name[i] = class_sig[i + 1] == '/' ? '.' : class_sig[i + 1];
      class_name[len_class_sig - 2] = 0;
      return class_name;
   }
}

/* -------------------------------------------------------------------

   The only way to identify objects seems to be the JNI method
   IsSameObject; although jobject is a pointer, the equality of two
   jobject's does not imply the identity of the represented Java
   objects.

   Furthermore, since we only have one object at hand to get an
   identifier needed for RTFL messages, we store all objects in a
   list, after creating a global reference (NewGlobalRef); the index
   in the list then identifies the object. This foils the garbage
   collection, but for short debugging sessions, this should be
   acceptable.

   ---------------------------------------------------------------------- */

static size_t reg_objects_size = 0, reg_objects_alloc_size;
static jobject *reg_objects = NULL;

size_t object_index (JNIEnv* jni, jobject object)
{
   size_t i;
   for (i = 0; i < reg_objects_size; i++)
      if ((*jni)->IsSameObject (jni, object, reg_objects[i]))
         return i;

   reg_objects_size++;
   if (reg_objects == NULL) {
      reg_objects_alloc_size = 1;
      reg_objects =
         (jobject*)malloc (reg_objects_alloc_size * sizeof (jobject));
   } else {
      reg_objects_alloc_size <<= 1;
      reg_objects =
         (jobject*)realloc (reg_objects,
                            reg_objects_alloc_size * sizeof (jobject));
   }
   
   i = reg_objects_size - 1;
   reg_objects[i] = (*jni)->NewGlobalRef (jni, object);
   return i;
}

void fill_object_buf (JNIEnv* jni, char *object_buf, jobject object)
{
   if (object)
      snprintf (object_buf, SIZE_OBJECT_BUF, "%ld", object_index (jni, object));
   else
      strcpy (object_buf, "null");
}

// Copied from "debug_rtfl.hh".
void rtfl_print (const char *module, const char *version,
                 const char *file, int line, const char *fmt, ...)
{
   // "\n" at the beginning just in case that the previous line is not
   // finished yet.
   printf ("\n[rtfl-%s-%s]%s:%d:pid(todo):", module, version, file, line);

   va_list args;
   va_start (args, fmt);

   int i;
   for (i = 0; fmt[i]; i++) {
      int n, j;
      void *p;
      char *s;

      switch (fmt[i]) {
      case 'd':
         n = va_arg(args, int);
         printf ("%d", n);
         break;

      case 'p':
         p = va_arg(args, void*);
         printf ("%p", p);
         break;

      case 's':
         s = va_arg (args, char*);
         for (j = 0; s[j]; j++) {
            if (s[j] == ':' || s[j] == '\\')
               putchar ('\\');
            putchar (s[j]);
         }
         break;

      case 'q':
         s = va_arg (args, char*);
         for (j = 0; s[j]; j++) {
            if (s[j] == ':' || s[j] == '\\')
               putchar ('\\');
            else if (s[j] == '\"')
               printf ("\\\\"); // a quoted quoting character
            putchar (s[j]);
         }
         break;

      case 'c':
         n = va_arg(args, int);
         printf ("#%06x", n);
         break;

      default:
         putchar (fmt[i]);
         break;
      }
   }

   va_end (args);

   putchar ('\n');
   fflush (stdout);
}
