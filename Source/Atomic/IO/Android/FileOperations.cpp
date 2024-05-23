#include "./FileOperations.h"
#include "../Log.h"
extern "C" {
    JNIEnv* Android_JNI_GetEnv();
}
namespace REngine {
    bool file_operations_file_exists(const ea::string& file_path) {
        JNIEnv* env = Android_JNI_GetEnv();

        jclass klass = env->FindClass("com/rengine/FileOperations");
        if(klass == nullptr) {
            ATOMIC_LOGERROR("Not found 'com.rengine.FileOperations' class.");
            return false;
        }

        jmethodID method_id = env->GetStaticMethodID(klass, "fileExists", "(Ljava/lang/String;)Z");
        if(method_id == nullptr) 
        {
            ATOMIC_LOGERROR("Not found 'com.rengine.FileOperations.fileExists' function.");
            return false;
        }

        jstring str = env->NewStringUTF(file_path.c_str());
        jboolean result = env->CallStaticBooleanMethod(klass, method_id, str);
        return static_cast<bool>(result);
    }

    const ea::string& file_operations_get_docs_dir() {
        JNIEnv* env = Android_JNI_GetEnv();

        jclass klass = env->FindClass("com/rengine/FileOperations");
        if(klass == nullptr) {
            ATOMIC_LOGERROR("Not found 'com.rengine.FileOperations' class.");
            return "";
        }

        jmethodID method_id = env->GetStaticMethodID(klass, "getDocumentsDir", "()Ljava/lang/String;");
        if(method_id == nullptr) 
        {
            ATOMIC_LOGERROR("Not found 'com.rengine.FileOperations.getDocumentsDir' function.");
            return "";
        }

        jstring str = static_cast<jstring>(env->CallStaticObjectMethod(klass, method_id));
        const char* str_ptr = env->GetStringUTFChars(str, nullptr);
        ea::string result(str_ptr);

        env->ReleaseStringUTFChars(str, str_ptr);
        return result;
    }
}