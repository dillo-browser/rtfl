#include <string.h>

#include "method.h"
#include "config.h"
#include "misc.h"

static bool handle_method (jvmtiEnv *jvmti, JNIEnv* jni, jthread thread,
                          jmethodID method, char **object, char **method_name,
                          char **class_name)
{
   jvmtiError error;
   char *class_sig = NULL, object_buf[SIZE_OBJECT_BUF];
   jclass klass;
   bool success = FALSE;
   
   *object = *method_name = *class_name = NULL;
   
   if ((error =
        (*jvmti)->GetMethodName (jvmti, method, method_name, NULL, NULL))
       != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error, "GetMethodName");
   else if ((error =
             (*jvmti)->GetMethodDeclaringClass (jvmti, method, &klass))
            != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error, "GetMethodDeclaringClass");
   else if ((error =
             (*jvmti)->GetClassSignature (jvmti, klass, &class_sig, NULL))
            != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error, "GetClassSignature");
   else {
      if ((*class_name = get_class_name_from_sig (class_sig, TRUE)) &&
          include_class (*class_name)) {
         jint numlocals;
         jvmtiLocalVariableEntry *locals;
         if ((error = (*jvmti)->GetLocalVariableTable (jvmti, method,
                                                       &numlocals, &locals))
             != JVMTI_ERROR_NONE)
            jvmti_error (jvmti, error, "GetLocalVariableTable");
         else {
            jint j, this_slot = -1;
            for (j = 0; j < numlocals && this_slot == -1; j++)
               if (strcmp (locals[j].name, "this") == 0)
                  this_slot = locals[j].slot;

            jobject this_obj;
            
            if (this_slot == -1) {
               this_obj = NULL;
               success = TRUE;
            } else {
               if ((error = (*jvmti)->GetLocalObject (jvmti, thread, 0,
                                                      this_slot, &this_obj))
                   != JVMTI_ERROR_NONE) {
                  jvmti_error (jvmti, error, "GetLocalObject");
               } else
                  success = TRUE;
            }

            if (success) {
               fill_object_buf (jni, object_buf, this_obj);
               *object = strdup (object_buf);
            }
         }
      }
   }

   jvmti_dealloc (jvmti, class_sig);

   return success;
}

static void handle_method_free (jvmtiEnv *jvmti, char *object,
                                char *method_name, char *class_name)
{
   simple_free (object);
   jvmti_dealloc (jvmti, method_name);
   simple_free (class_name);
}

void JNICALL method_entry (jvmtiEnv *jvmti, JNIEnv* jni, jthread thread,
                           jmethodID method)
{
   char *object, *method_name, *class_name;

   if (handle_method (jvmti, jni, thread, method, &object, &method_name,
                      &class_name)) {
      if (strcmp (method_name, "<init>") == 0)
         RTFL_OBJ_PRINT ("create", "s:s", object, class_name);
      else
         RTFL_OBJ_PRINT ("enter", "s:s:d:s:",
                         object, "", 0, method_name, class_name);
   }

   handle_method_free (jvmti, object, method_name, class_name);
}

void JNICALL method_exit (jvmtiEnv *jvmti, JNIEnv* jni, jthread thread,
                          jmethodID method, jboolean was_popped_by_exception,
                          jvalue return_value)
{
   char *object, *method_name, *class_name;

   if (handle_method (jvmti, jni, thread, method, &object, &method_name,
                      &class_name)) {
      if (strcmp (method_name, "<init>") != 0)
         // Hopefully, this is the correct method.
         RTFL_OBJ_PRINT ("leave", "s", object);
   }

   handle_method_free (jvmti, object, method_name, class_name);
}
