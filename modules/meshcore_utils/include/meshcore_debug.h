/*
 * Copyright (C) 2026 Université Grenoble Alpes - CSUG - LIG
 */

/*
 * Utils for MeshCore mesh network
 */

#ifndef _MESHCORE_UTILS_DEBUG_H_
#define _MESHCORE_UTILS_DEBUG_H_

#ifdef RIOT_VERSION

#define ENABLE_DEBUG		ENABLE_DEBUG_MESHCORE
#include "debug.h"

#define MESH_DEBUG_PRINT(F, ...)    DEBUG_PRINT(F, ...)
#define MESH_DEBUG_PRINTLN(F, ...)  DEBUG_PRINTLN(F, ...)

#else

#if MESH_DEBUG && ARDUINO
  #include <Arduino.h>
  #define MESH_DEBUG_PRINT(F, ...) Serial.printf("DEBUG: " F, ##__VA_ARGS__)
  #define MESH_DEBUG_PRINTLN(F, ...) Serial.printf("DEBUG: " F "\n", ##__VA_ARGS__)
#else
  #define MESH_DEBUG_PRINT(...) {}
  #define MESH_DEBUG_PRINTLN(...) {}
#endif

#if BRIDGE_DEBUG && ARDUINO
#define BRIDGE_DEBUG_PRINTLN(F, ...) Serial.printf("%s BRIDGE: " F, getLogDateTime(), ##__VA_ARGS__)
#else
#define BRIDGE_DEBUG_PRINTLN(...) {}
#endif

#endif // RIOT_VERSION

#endif