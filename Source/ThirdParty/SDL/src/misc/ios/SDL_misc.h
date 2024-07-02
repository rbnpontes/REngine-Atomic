#ifndef SDL_MISC_H
#define SDL_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

    extern char *SDL_IOS_GetDocumentsDir();
    extern char *SDL_IOS_GetResourceDir();
    extern void SDL_IOS_LogMessage(const char *message);

#ifdef __cplusplus
}
#endif


#endif
