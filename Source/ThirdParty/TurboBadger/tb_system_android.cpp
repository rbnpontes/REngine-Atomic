// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"

#ifdef TB_SYSTEM_ANDROID

#include <android/log.h>
#include <sys/time.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/configuration.h>

#ifdef TB_RUNTIME_DEBUG_INFO

#define  LOG_TAG    "TB"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR ,LOG_TAG, __VA_ARGS__)

void TBDebugOut(const char *str)
{
    LOGI(str);
}

#endif // TB_RUNTIME_DEBUG_INFO

extern "C" {
    // access SDL JNIEnv
    JNIEnv* Android_JNI_GetEnv();
}

AAssetManager *g_pManager = NULL;

AAssetManager* GetAssetManager() 
{
    if(g_pManager)
        return g_pManager;
    
    JNIEnv* env = Android_JNI_GetEnv();

    jclass klass = env->FindClass("com/rengine/EngineGlobals");
    assert(klass != NULL && "Not found com.rengine.EngineGlobals java class");

    jmethodID method_id = env->GetStaticMethodID(klass, "getAssetManager", "()Landroid/content/res/AssetManager;");
    assert(method_id != NULL && "Not found com.rengine.EngineGlobals.getAssetManager method");

    jobject asset_mgr = env->CallStaticObjectMethod(klass, method_id);

    return g_pManager = AAssetManager_fromJava(env, asset_mgr);
}

namespace tb {
    // == TBSystem ========================================

    double TBSystem::GetTimeMS()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        return now.tv_usec / 1000 + now.tv_sec * 1000;
    }

    void TBSystem::RescheduleTimer(double fire_time)
    {
    }

    int TBSystem::GetLongClickDelayMS()
    {
        return 500;
    }

    int TBSystem::GetPanThreshold()
    {
        return 5 * GetDPI() / 120;
    }

    int TBSystem::GetPixelsPerLine()
    {
        return 40 * GetDPI() / 120;
    }

    int TBSystem::GetDPI()
    {
        AConfiguration *config = AConfiguration_new();
        AConfiguration_fromAssetManager(config, GetAssetManager());
        int32_t density = AConfiguration_getDensity(config);
        AConfiguration_delete(config);
        if (density == 0 || density == ACONFIGURATION_DENSITY_NONE)
            return 120;
        return density;
    }
}
#endif // TB_SYSTEM_ANDROID
