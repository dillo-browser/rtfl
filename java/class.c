#include "class.h"
#include "config.h"
#include "misc.h"

void JNICALL class_prepare(jvmtiEnv *jvmti, JNIEnv* jni, jthread thread,
                           jclass klass)
{
   jvmtiError error;
   char *class_sig = NULL, *class_name = NULL;

   if ((error = (*jvmti)->GetClassSignature (jvmti, klass, &class_sig, NULL))
       != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error, "GetClassSignature");
   else {
      if ((class_name = get_class_name_from_sig (class_sig, TRUE)) &&
          include_class (class_name)) {
         jint field_count;
         jfieldID* fields;
         if ((error = (*jvmti)->GetClassFields (jvmti, klass, &field_count,
                                                &fields))
             != JVMTI_ERROR_NONE)
            jvmti_error (jvmti, error, "GetClassFields");
         else {
            int i;
            for (i = 0; i < field_count; i++) {
               if ((error =
                    (*jvmti)->SetFieldModificationWatch (jvmti, klass,
                                                         fields[i]))
                   != JVMTI_ERROR_NONE)
                  jvmti_error (jvmti, error, "SetFieldModificationWatch");
            }
         }
      }
   }

   jvmti_dealloc (jvmti, class_sig);
   simple_free (class_name);
   
}
