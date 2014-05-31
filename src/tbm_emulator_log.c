#include "tbm_emulator_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

static const char *g_log_level_to_str[] =
{
    "OFF",
    "ERROR",
    "INFO",
    "DEBUG"
};

static pthread_once_t g_log_init = PTHREAD_ONCE_INIT;
static int g_debug_enabled = 0;

static void tbm_emulator_log_init_once(void)
{
    char *debug_enabled_str = getenv("TBM_EMULATOR_DEBUG");
    g_debug_enabled = debug_enabled_str ? atoi(debug_enabled_str) : 0;
}

static void tbm_emulator_log_init(void)
{
    pthread_once(&g_log_init, tbm_emulator_log_init_once);
}

static void tbm_emulator_log_print_current_time(void)
{
    char buff[128];
    struct tm tm;
    struct timeval tv = { 0, 0 };
    time_t ti;

    gettimeofday(&tv, NULL);

    ti = tv.tv_sec;

    localtime_r(&ti, &tm);
    strftime(buff, sizeof(buff),
             "%H:%M:%S", &tm);
    fprintf(stderr, "%s", buff);
}

void tbm_emulator_log_event(tbm_emulator_log_level log_level,
                            const char *func,
                            int line,
                            const char *format, ...)
{
    va_list args;

    tbm_emulator_log_init();

    tbm_emulator_log_print_current_time();
    fprintf(stderr,
            " %-5s [%u] %s:%d",
            g_log_level_to_str[log_level],
            getpid(),
            func,
            line);
    if (format) {
        va_start(args, format);
        fprintf(stderr, " - ");
        vfprintf(stderr, format, args);
        va_end(args);
    }
    fprintf(stderr, "\n");
}

int tbm_emulator_log_is_debug_enabled()
{
    tbm_emulator_log_init();

    return g_debug_enabled;
}
