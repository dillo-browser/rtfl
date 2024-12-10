#include <string.h>
#include <stdlib.h>

#include "field.h"
#include "config.h"
#include "misc.h"

/* ----------------------------------------------------------------------

   A sketch on how field modifications could be processed:

   A field modification results in a "set" or in an "assoc"
   command. Neither is the field name *necessarily* the name of the "set"
   command (or "assoc", if it would support names), nor is the field
   value necessarily the respective value. Instead, the following rules
   are used to determine the structure of the fields.

   (A) If the value type is "printable" (i. e. primitive, or a wrapper
   class for a primitive type, or java.lang.String, or an enum (more
   cases?)), then a set command is printed, with the name and the value
   of the field.

   (B) If the value is a visible object, and it is not configured as
   sub-structure (see below), an "assoc" command is printed, between this
   object and the value object. (As soon as associations get a name, the
   field name is used for this.)

   (C) If the value is an invisible object, or a visible object which is
   configured as sub-structure, regard it as structured, according to the
   rules which follow.

   Structured values are divided into sub-values with sub-names; the
   field name, followed by a dot, is used as base-name; all sub-values
   and sub-names are processed recursively according to the same rules,
   while the base-name is eventually prepended to all sub-names.

   (C1) If the value is of type java.util.Map, and the keys are printable
   (see (A)), use the keys as sub-names, the values as sub-values. ("The
   keys are printable": does this mean that the type for the keys is a
   class which is generally printable?)

   (C2) If the value is of type java.util.Map, and the keys are not
   printable (see also C1), regard the value as unsorted list of
   elements, which are themselves structured sub-values
   (java.util.Map.Entry?).

   (C3) If the value is of type java.util.List, use the indices as
   sub-names, and the elements as sub-values.

   (C4) If the value is an unsorted collection, i. e. of type
   java.util.Collection, but not of java.util.Map or java.util.List,
   apply a random order and proceed as in (C4). (Unsorted collections are
   not really supported in RTFL.)

   The rules (C1) to (C4) apply not when configured otherwise ("do not
   show logical structure").

   (C5) Otherwise (not of type java.util.Collection or java.util.Map),
   examine fields of the value; regard the names of its fields as
   sub-names; their values as sub-values;

   (Alternative: Java Beans?)

   For simplification, ignore C1 to C4 in the first place and focus on
   C5; then implement C1 to C4 by and by.
   
   ---------------------------------------------------------------------- */

static void print_field (jvmtiEnv *jvmti, JNIEnv* jni, jclass field_klass,
                         jobject object, jfieldID field, jvalue value,
                         char *base_name);
static void print_string_field (jvmtiEnv *jvmti, JNIEnv* jni, char *object_str,
                                char *base_name, char *field_name,
                                jstring string);

void JNICALL field_modification (jvmtiEnv *jvmti, JNIEnv* jni, jthread thread,
                                 jmethodID method, jlocation location,
                                 jclass field_klass, jobject object,
                                 jfieldID field, char signature_type,
                                 jvalue new_value)
{
   // The class name is irrelevant here, since classes are already
   // filtered inclass_prepare(), where SetFieldModificationWatch()
   // is called.

   print_field (jvmti, jni, field_klass, object, field, new_value, "");
}

void print_field (jvmtiEnv *jvmti, JNIEnv* jni, jclass field_klass,
                  jobject object, jfieldID field, jvalue value, char *base_name)
{
   jvmtiError error;
   char object_buf1[SIZE_OBJECT_BUF], object_buf2[SIZE_OBJECT_BUF];
   char *class_name = NULL, *field_name = NULL, *field_sig = NULL;

   if ((error = (*jvmti)->GetFieldName (jvmti, field_klass, field, &field_name,
                                        &field_sig, NULL))
       != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error, "GetFieldName");
   else {
      //printf ("==> %s - %s\n", field_name, field_sig);

      fill_object_buf (jni, object_buf1, object);

      if ((class_name = get_class_name_from_sig (field_sig, FALSE))) {
         if (include_class (class_name)) {
            // The field represents an instance of a class which is also
            // included: this is an association, not a simple field.
            // ("field_name" could be used here, as soon as associations get
            // a name.)
            
            if (value.l) {
               fill_object_buf (jni, object_buf2, value.l);
               RTFL_OBJ_PRINT ("assoc", "s:s", object_buf1, object_buf2);
            }
         } else {
            if (strcmp (class_name, "java.lang.String")) {
               print_string_field (jvmti, jni, object_buf1, base_name,
                                   field_name, (jstring)(value.l));
                                   
            }                                  
         }
      }
   }
   
   jvmti_dealloc (jvmti, field_name);
   jvmti_dealloc (jvmti, field_sig);
   simple_free (class_name);
}

void print_string_field (jvmtiEnv *jvmti, JNIEnv* jni, char *object_str,
                         char *base_name, char *field_name, jstring string)
{
   if (string == NULL)
      RTFL_OBJ_PRINT ("set", "s:ss:s", object_str, base_name, field_name,
                      "null");
   else {
      // TODO: JNI calls somehow cause an abortion. Just some test
      // code.

      //jsize len = (*jni)->GetStringLength (jni, string);
      //const jchar *chars = (*jni)->GetStringChars (jni, string, NULL);

      //char *chars0 = (char*)malloc (len + 1);
      //memcpy (chars0, chars, len);
      //chars0[len] = 0;

      char *chars0 = "?\"?\"?";
      
      RTFL_OBJ_PRINT ("set", "s:ss:\"q\"", object_str, base_name, field_name,
                      chars0);
      
      //free (chars0);
      //(*jni)->ReleaseStringChars (jni, string, chars);
   }
}
