

#ifndef ENGINE_API_H
#define ENGINE_API_H

#ifdef _MSC_VER

    #pragma warning(disable: 4251)
    #pragma warning(disable: 4275)

    #ifdef ENGINE_STATIC_DEFINE
        #ifndef ENGINE_API
            #define ENGINE_API
        #endif
        #define ENGINE_NO_EXPORT
    #else
    #  ifndef ENGINE_API
    #    ifdef ENGINE_EXPORTS
            /* We are building this library */
    #      define ENGINE_API __declspec(dllexport)
    #    else
            /* We are using this library */
    #      define ENGINE_API __declspec(dllimport)
    #    endif
    #  endif

    #  ifndef ENGINE_NO_EXPORT
    #    define ENGINE_NO_EXPORT 
    #  endif
    #endif

    #ifndef ENGINE_DEPRECATED
    #  define ENGINE_DEPRECATED __declspec(deprecated)
    #endif

    #ifndef ENGINE_DEPRECATED_EXPORT
    #  define ENGINE_DEPRECATED_EXPORT ENGINE_API ENGINE_DEPRECATED
    #endif

    #ifndef ENGINE_DEPRECATED_NO_EXPORT
        #define ENGINE_DEPRECATED_NO_EXPORT ENGINE_NO_EXPORT ENGINE_DEPRECATED
    #endif

    #define DEFINE_NO_DEPRECATED 0
    #if DEFINE_NO_DEPRECATED
        #define ENGINE_NO_DEPRECATED
    #endif


    #ifndef ENGINE_DEFINE_PLUGIN_SIGNATURE
        #define ENGINE_DEFINE_PLUGIN_SIGNATURE extern "C" __declspec(dllexport) void* rengine_plugin_entrypoint();
    #endif
#else

    #ifdef ENGINE_STATIC_DEFINE
        #ifndef ENGINE_API
            #define ENGINE_API
        #endif

        #define ENGINE_NO_EXPORT
    #else
        #ifndef ENGINE_API
            #ifdef ENGINE_EXPORTS
                /* We are building this library */
                #define ENGINE_API __attribute__((visibility("default")))
            #else
                /* We are using this library */
                #define ENGINE_API __attribute__((visibility("default")))
            #endif
        #endif

        #ifndef ENGINE_NO_EXPORT
            #define ENGINE_NO_EXPORT __attribute__((visibility("hidden")))
        #endif
    #endif

    #ifndef ENGINE_DEPRECATED
        #define ENGINE_DEPRECATED __attribute__ ((__deprecated__))
    #endif

    #ifndef ENGINE_DEPRECATED_EXPORT
        #define ENGINE_DEPRECATED_EXPORT ENGINE_API ENGINE_DEPRECATED
    #endif

    #ifndef ENGINE_DEPRECATED_NO_EXPORT
        #define ENGINE_DEPRECATED_NO_EXPORT ENGINE_NO_EXPORT ENGINE_DEPRECATED
    #endif

    #define DEFINE_NO_DEPRECATED 0
    #if DEFINE_NO_DEPRECATED
        #define ENGINE_NO_DEPRECATED
    #endif

    #ifndef ENGINE_DEFINE_PLUGIN_SIGNATURE
        #define ENGINE_DEFINE_PLUGIN_SIGNATURE extern "C" __attribute__((visibility("default"))) void* rengine_plugin_entrypoint();
    #endif
#endif

#define RENGINE_API ENGINE_API
// TODO: remove this when all Atomic brand is removed
// Don't break current implemenations
#define ATOMIC_API ENGINE_API
#endif

