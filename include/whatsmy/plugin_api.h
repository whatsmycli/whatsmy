// whatsmycli - Plugin API Definition
// Copyright (C) 2025 enXov
// Licensed under GPLv3

#ifndef WHATSMY_PLUGIN_API_H
#define WHATSMY_PLUGIN_API_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Plugin API Version
 * Update this when making breaking changes to the plugin interface
 */
#define WHATSMY_PLUGIN_API_VERSION 1

/**
 * Plugin entry point
 * 
 * Every plugin must implement this function.
 * 
 * @return 0 on success, non-zero error code on failure
 * 
 * Example implementation:
 * 
 * extern "C" {
 *     int plugin_run() {
 *         std::cout << "Hello from plugin!" << std::endl;
 *         return 0;
 *     }
 * }
 */
int plugin_run();

#ifdef __cplusplus
}
#endif

#endif // WHATSMY_PLUGIN_API_H

