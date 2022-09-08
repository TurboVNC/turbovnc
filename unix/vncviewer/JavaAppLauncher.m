/*
 * Copyright 2012, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2013, 2017-2019, 2022 D. R. Commander. All rights reserved.
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#import <Cocoa/Cocoa.h>
#include <dlfcn.h>
#include <jni.h>

#if !defined(MAC_OS_X_VERSION_10_12) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
#define NSAlertStyleCritical NSCriticalAlertStyle
#endif

#define JAVA_LAUNCH_ERROR "JavaLaunchError"

#define JVM_RUNTIME_KEY "JVMRuntime"
#define JVM_MAIN_CLASS_NAME_KEY "JVMMainClassName"
#define JVM_OPTIONS_KEY "JVMOptions"
#define JVM_ARGUMENTS_KEY "JVMArguments"

#define UNSPECIFIED_ERROR "An unknown error occurred."

#define APP_ROOT_PREFIX "$APP_ROOT"

#define LIBJLI_DYLIB "/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home/lib/jli/libjli.dylib"

typedef int (JNICALL *JLI_Launch_t)(int argc, char **argv,
                                    int jargc, const char **jargv,
                                    int appclassc, const char **appclassv,
                                    const char *fullversion,
                                    const char *dotversion,
                                    const char *pname, const char *lname,
                                    jboolean javaargs, jboolean cpwildcard,
                                    jboolean javaw, jint ergo);

int launch(char *);

char **jargv = NULL;
int jargc = 0;
bool firstTime = true;

int main(int argc, char *argv[]) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  int result;
  @try {
    // DRC: I have no idea why this program re-enters itself, but that's
    // why the "firstTime" check is necessary.
    if (argc > 1 && !jargv && firstTime) {
      jargv = &argv[1];
      jargc = argc - 1;
    }
    firstTime = false;
    launch(argv[0]);
    result = 0;
  } @catch (NSException *exception) {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setAlertStyle:NSAlertStyleCritical];
    [alert setMessageText:[exception reason]];
    [alert runModal];

    result = 1;
  }

  [pool drain];

  return result;
}

int launch(char *commandName) {
  // Get the main bundle
  NSBundle *mainBundle = [NSBundle mainBundle];

  // Set the working directory to the user's home directory
  chdir([NSHomeDirectory() UTF8String]);

  // Get the main bundle's info dictionary
  NSDictionary *infoDictionary = [mainBundle infoDictionary];

  // Locate the JLI_Launch() function
  const char *libjliPath = NULL;
  NSString *runtime = [infoDictionary objectForKey:@JVM_RUNTIME_KEY];
  if (runtime != nil) {
    void *tmpLib = NULL;
    NSString *runtimePath = [[[NSBundle mainBundle] builtInPlugInsPath]
                             stringByAppendingPathComponent:runtime];

    libjliPath = [[runtimePath stringByAppendingPathComponent:@"Contents/Home/lib/jli/libjli.dylib"]
                  fileSystemRepresentation];
    if ((tmpLib = dlopen(libjliPath, RTLD_LAZY)) == nil)
      libjliPath = [[runtimePath stringByAppendingPathComponent:@"Contents/Home/lib/libjli.dylib"]
                    fileSystemRepresentation];
    if (tmpLib) dlclose(tmpLib);
  } else {
    FILE *file = NULL;
    char path[1024] = "\0";
    NSString *env =
      [[[NSProcessInfo processInfo]environment]objectForKey:@"JAVA_HOME"];

    if (env != nil && [env length] > 0) {
      void *tmpLib = NULL;

      libjliPath =
        [[env stringByAppendingPathComponent:@"jre/lib/jli/libjli.dylib"]
         fileSystemRepresentation];
      if ((tmpLib = dlopen(libjliPath, RTLD_LAZY)) == nil) {
        libjliPath =
          [[env stringByAppendingPathComponent:@"lib/jli/libjli.dylib"]
           fileSystemRepresentation];
        if ((tmpLib = dlopen(libjliPath, RTLD_LAZY)) == nil)
          libjliPath =
            [[env stringByAppendingPathComponent:@"lib/libjli.dylib"]
             fileSystemRepresentation];
      }
      if (tmpLib) dlclose(tmpLib);
    } else if ((file = popen("/usr/libexec/java_home", "r")) != NULL &&
               fgets(path, 1024, file) != NULL && strlen(path) > 0) {
      void *tmpLib = NULL;
      NSString *nsPath;

      if (path[strlen(path) - 1] == '\n') path[strlen(path) - 1] = 0;
      nsPath = [NSString stringWithUTF8String:path];

      libjliPath =
        [[nsPath stringByAppendingPathComponent:@"jre/lib/jli/libjli.dylib"]
         fileSystemRepresentation];
      if ((tmpLib = dlopen(libjliPath, RTLD_LAZY)) == nil) {
        libjliPath =
          [[nsPath stringByAppendingPathComponent:@"lib/jli/libjli.dylib"]
           fileSystemRepresentation];
        if ((tmpLib = dlopen(libjliPath, RTLD_LAZY)) == nil)
          libjliPath =
            [[nsPath stringByAppendingPathComponent:@"lib/libjli.dylib"]
             fileSystemRepresentation];
      }
      if (tmpLib) dlclose(tmpLib);
    } else
      libjliPath = LIBJLI_DYLIB;

    if (file) pclose(file);
  }

  void *libJLI = dlopen(libjliPath, RTLD_LAZY);

  JLI_Launch_t jli_LaunchFxnPtr = NULL;
  if (libJLI != NULL)
    jli_LaunchFxnPtr = dlsym(libJLI, "JLI_Launch");

  if (jli_LaunchFxnPtr == NULL)
    [[NSException exceptionWithName:@JAVA_LAUNCH_ERROR
      reason:NSLocalizedString(@"JRELoadError", @UNSPECIFIED_ERROR)
      userInfo:nil] raise];

  // Get the main class name
  NSString *mainClassName =
    [infoDictionary objectForKey:@JVM_MAIN_CLASS_NAME_KEY];
  if (mainClassName == nil)
    [[NSException exceptionWithName:@JAVA_LAUNCH_ERROR
      reason:NSLocalizedString(@"MainClassNameRequired", @UNSPECIFIED_ERROR)
      userInfo:nil] raise];

  // Set the class path
  NSString *mainBundlePath = [mainBundle bundlePath];
  NSString *javaPath =
    [mainBundlePath stringByAppendingString:@"/Contents/Resources/Java"];
  NSMutableString *classPath =
    [NSMutableString stringWithFormat:@"-Djava.class.path=%@/Classes",
     javaPath];

  NSFileManager *defaultFileManager = [NSFileManager defaultManager];
  NSArray *javaDirectoryContents =
    [defaultFileManager contentsOfDirectoryAtPath:javaPath error:nil];
  if (javaDirectoryContents == nil)
    [[NSException exceptionWithName:@JAVA_LAUNCH_ERROR
      reason:NSLocalizedString(@"JavaDirectoryNotFound", @UNSPECIFIED_ERROR)
      userInfo:nil] raise];

  for (NSString *file in javaDirectoryContents) {
    if ([file hasSuffix:@".jar"])
      [classPath appendFormat:@":%@/%@", javaPath, file];
  }

  // Set the library path
  NSString *libraryPath = [NSString stringWithFormat:@"-Djava.library.path=%@/Contents/Resources/Native", mainBundlePath];

  // Get the VM options
  NSArray *options = [infoDictionary objectForKey:@JVM_OPTIONS_KEY];
  if (options == nil)
    options = [NSArray array];

  // Get the application arguments
  NSArray *arguments = [infoDictionary objectForKey:@JVM_ARGUMENTS_KEY];
  if (arguments == nil)
    arguments = [NSArray array];

  // Initialize the arguments to JLI_Launch()
  int argc = 1 + [options count] + 2 + [arguments count] + 1 + jargc;
  char *argv[argc];

  int i = 0;
  argv[i++] = commandName;
  argv[i++] = strdup([classPath UTF8String]);
  argv[i++] = strdup([libraryPath UTF8String]);

  for (NSString *option in options) {
    option = [option stringByReplacingOccurrencesOfString:@APP_ROOT_PREFIX
              withString:[mainBundle bundlePath]];
    argv[i++] = strdup([option UTF8String]);
  }

  argv[i++] = strdup([mainClassName UTF8String]);

  for (NSString *argument in arguments) {
    argument = [argument stringByReplacingOccurrencesOfString:@APP_ROOT_PREFIX
                withString:[mainBundle bundlePath]];
    argv[i++] = strdup([argument UTF8String]);
  }

  if (jargc > 0 && jargv) {
    int j;

    for (j = 0; j < jargc; j++) {
      if (!strncmp(jargv[j], "-psn", 4)) {
        argc--;
        continue;
      }
      argv[i++] = jargv[j];
    }
  }

  // Invoke JLI_Launch()
  return jli_LaunchFxnPtr(argc, argv, 0, NULL, 0, NULL, "", "", "java", "java",
                          FALSE, FALSE, FALSE, 0);
}
