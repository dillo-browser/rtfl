#ifndef __JAVA_METHOD_H__
#define __JAVA_METHOD_H__

#include <jvmti.h>

void JNICALL method_entry (jvmtiEnv *jvmti, JNIEnv* jni, jthread thread,
                           jmethodID method);
void JNICALL method_exit (jvmtiEnv *jvmti, JNIEnv* jni, jthread thread,
                          jmethodID method, jboolean was_popped_by_exception,
                          jvalue return_value);

#endif /* __JAVA_METHOD_H__ */
