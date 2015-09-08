/*
 * libtbm-emulator log
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

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
