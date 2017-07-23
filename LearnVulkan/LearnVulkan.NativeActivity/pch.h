//
// pch.h
// Header for standard system include files.
//
// Used by the build system to generate the precompiled header. Note that no
// pch.cpp is needed and the pch.h is automatically included in all cpp files
// that are part of the project
//
#include <cassert>

#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <android/sensor.h>

#include <android/log.h>
#include "android_native_app_glue.h"

#define VK_ALLOC_CALLBACK (VkAllocationCallbacks*)NULL
#define VK_VALIDATION_RESULT(res) { assert(res == VK_SUCCESS); }
