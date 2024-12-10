#include <jvmti.h>
#include <string.h>
#include <stdlib.h>

#include "class.h"
#include "method.h"
#include "field.h"
#include "misc.h"

JNIEXPORT jint JNICALL Agent_OnLoad (JavaVM *jvm, char *options, void *reserved)
{
   rtfl_print ("obj", RTFL_OBJ_VERSION, "", 0, "s", "noident");

   jvmtiEnv *jvmti = NULL;

   (*jvm)->GetEnv (jvm, (void**)&jvmti, JVMTI_VERSION_1_0);

   jvmtiCapabilities capa;
   memset (&capa, 0, sizeof(jvmtiCapabilities));
   capa.can_generate_method_entry_events = 1;
   capa.can_generate_method_exit_events = 1;
   capa.can_access_local_variables = 1;
   capa.can_generate_field_modification_events = 1;

   jvmtiEventCallbacks callbacks;
   (void)memset(&callbacks, 0, sizeof(callbacks));
   callbacks.ClassPrepare = &class_prepare;
   callbacks.MethodEntry = &method_entry;
   callbacks.MethodExit = &method_exit;
   callbacks.FieldModification = &field_modification;
   
   jvmtiError error;  
   if ((error = (*jvmti)->AddCapabilities(jvmti, &capa)) != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error, "AddCapabilities");
   else if ((error =
              (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE,
                                                  JVMTI_EVENT_CLASS_PREPARE,
                                                  (jthread)NULL))
            != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error,
                   "SetEventNotificationMode (JVMTI_EVENT_CLASS_PREPARE)");
   else if ((error =
              (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE,
                                                  JVMTI_EVENT_METHOD_ENTRY,
                                                  (jthread)NULL))
            != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error,
                   "SetEventNotificationMode (JVMTI_EVENT_METHOD_ENTRY)");
   else if ((error =
              (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE,
                                                  JVMTI_EVENT_METHOD_EXIT,
                                                  (jthread)NULL))
            != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error,
                   "SetEventNotificationMode (JVMTI_EVENT_METHOD_EXIT)");
   else if ((error =
              (*jvmti)->SetEventNotificationMode
                           (jvmti, JVMTI_ENABLE, JVMTI_EVENT_FIELD_MODIFICATION,
                            (jthread)NULL)) != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error,
                   "SetEventNotificationMode (JVMTI_EVENT_FIELD_MODIFICATION");
   else if ((error = (*jvmti)->SetEventCallbacks(jvmti, &callbacks,
                                                 (jint)sizeof(callbacks)))
            != JVMTI_ERROR_NONE)
      jvmti_error (jvmti, error, "SetEventCallbacks");

   return JNI_OK;
}
