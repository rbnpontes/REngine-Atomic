#include "SDL_misc.h"
#include <Foundation/Foundation.h>
#import <asl.h>

char *SDL_IOS_GetDocumentsDir() {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    
    const char *cString = [documentsDirectory UTF8String];
    char *cStringCopy = strdup(cString); // Copy the string to ensure memory ownership
    return cStringCopy;
}

char *SDL_IOS_GetResourceDir() {
    NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
    
    const char *cString = [resourcePath UTF8String];
    char *cStringCopy = strdup(cString); // Copy the string to ensure memory ownership
    return cStringCopy;
}

void SDL_IOS_LogMessage(const char *message) {
    NSString *logMessage = [NSString stringWithUTF8String:message];
    asl_log(NULL, NULL, ASL_LEVEL_NOTICE, "%s", [logMessage UTF8String]);
}
