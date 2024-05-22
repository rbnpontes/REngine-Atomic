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
}