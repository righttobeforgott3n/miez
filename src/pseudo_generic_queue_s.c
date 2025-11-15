#if defined(_WIN32) || defined(_WIN64)
#include "pseudo_generic_queue_s_windows.c"

#elif defined(__linux__)
#include "pseudo_generic_queue_s_linux.c"

#else
#error                                                                         \
    "Unsupported platform! Add support for your platform in pseudo_generic_queue_s.c"

#endif
