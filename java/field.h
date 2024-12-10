#ifndef __JAVA_FIELD_H__
#define __JAVA_FIELD_H__

#include <jvmti.h>

void JNICALL field_modification (jvmtiEnv *jvmti, JNIEnv* jni, jthread thread,
                                 jmethodID method, jlocation location,
                                 jclass field_klass, jobject object,
                                 jfieldID field, char signature_type,
                                 jvalue new_value);

#endif /* __JAVA_FIELD_H__ */
