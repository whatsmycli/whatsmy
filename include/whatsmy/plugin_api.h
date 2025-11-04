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
 * 
 * Version History:
 *   1 - Initial API with plugin_run()
 *   2 - Added argument support with plugin_run(int argc, char* argv[])
 */
#define WHATSMY_PLUGIN_API_VERSION 2

/**
 * Plugin entry point with argument support
 * 
 * Every plugin must implement this function.
 * 
 * @param argc Number of arguments passed to the plugin
 * @param argv Array of argument strings
 *              argv[0] is the plugin name
 *              argv[1..argc-1] are additional arguments
 * 
 * @return 0 on success, non-zero error code on failure
 * 
 * Example implementation:
 * 
 * extern "C" {
 *     int plugin_run(int argc, char* argv[]) {
 *         std::cout << "Plugin called with " << argc << " arguments" << std::endl;
 *         
 *         // Print all arguments
 *         for (int i = 0; i < argc; i++) {
 *             std::cout << "  argv[" << i << "] = " << argv[i] << std::endl;
 *         }
 *         
 *         // Example: greeting plugin with optional name
 *         if (argc >= 2) {
 *             std::cout << "Hello to you too, " << argv[1] << "!" << std::endl;
 *         } else {
 *             std::cout << "Hello, World!" << std::endl;
 *         }
 *         
 *         return 0;
 *     }
 * }
 */
int plugin_run(int argc, char* argv[]);

#ifdef __cplusplus
}
#endif

#endif // WHATSMY_PLUGIN_API_H

