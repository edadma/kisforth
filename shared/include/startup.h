#ifndef STARTUP_H
#define STARTUP_H

// System initialization functions
void forth_system_init(void);

// Startup banner and messaging
void print_startup_banner(const char* platform_name);
void print_extensions_list(void);

#endif  // STARTUP_H