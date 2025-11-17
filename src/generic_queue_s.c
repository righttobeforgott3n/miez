#if defined(_WIN32) || defined(_WIN64)
#include "generic_queue_s_windows.c"

#elif defined(__linux__)
#include "generic_queue_s_linux.c"

#else
#error                                                                         \
    "Unsupported platform! Add support for your platform in generic_queue_s.c"

#endif
