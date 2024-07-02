#pragma
#include "../../Container/TypeTraits.h"
#include <jni.h>

namespace REngine {
    bool file_operations_file_exists(const ea::string& file_path);
    const ea::string& file_operations_get_docs_dir();
}