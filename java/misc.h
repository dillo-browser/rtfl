#ifndef __JAVA_MISC_H__
#define __JAVA_MISC_H__

#include <jvmti.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifndef __GNUC__
#  define  __attribute__(x)  /* nothing */
#endif

typedef enum { FALSE = 0, TRUE = 1 } bool;
enum { SIZE_OBJECT_BUF = 32 };

bool str_starts_with (const char *haystack, const char *needle);

void jvmti_error (jvmtiEnv *jvmti, jvmtiError error, const char *fmt, ...)
   __attribute__((format(printf, 3, 4)));
void other_error (const char *fmt, ...) __attribute__((format(printf, 1, 2)));

void jvmti_dealloc (jvmtiEnv *jvmti, void *mem);
void simple_free (void *mem);

char *get_class_name_from_sig (const char *class_sig, bool expect_class);

size_t object_index (JNIEnv* jni, jobject object);
void fill_object_buf (JNIEnv* jni, char *object_buf, jobject object);

void rtfl_print (const char *module, const char *version,
                 const char *file, int line, const char *fmt, ...);

#define RTFL_PRINT(module, version, cmd, fmt, ...) \
   rtfl_print (module, version, "", 1, "s:" fmt, cmd, __VA_ARGS__)

#define RTFL_OBJ_VERSION "1.0"

#define RTFL_OBJ_PRINT(cmd, fmt, ...) \
   RTFL_PRINT ("obj", RTFL_OBJ_VERSION, cmd, fmt, __VA_ARGS__)

#endif /* __JAVA_MISC_H__ */
