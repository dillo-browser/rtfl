#ifndef __JAVA_CLASS_H__
#define __JAVA_CLASS_H__

#include <jvmti.h>

void JNICALL class_prepare(jvmtiEnv *jvmti, JNIEnv* jni, jthread thread,
                           jclass klass);

#endif /* __JAVA_CLASS_H__ */
