#ifndef _TBM_EMULATOR_LOG_H
#define _TBM_EMULATOR_LOG_H

typedef enum
{
    tbm_emulator_log_level_off = 0,
    tbm_emulator_log_level_error = 1,
    tbm_emulator_log_level_info = 2,
    tbm_emulator_log_level_debug = 3,
} tbm_emulator_log_level;

void tbm_emulator_log_event(tbm_emulator_log_level log_level,
                            const char *func,
                            int line,
                            const char *format, ...);

int tbm_emulator_log_is_debug_enabled();

#define TBM_EMULATOR_LOG_DEBUG(format, ...) \
    do { \
        if (tbm_emulator_log_is_debug_enabled()) { \
            tbm_emulator_log_event(tbm_emulator_log_level_debug, __FUNCTION__, __LINE__, format,##__VA_ARGS__); \
        } \
    } while(0)

#define TBM_EMULATOR_LOG_INFO(format, ...) tbm_emulator_log_event(tbm_emulator_log_level_info, __FUNCTION__, __LINE__, format,##__VA_ARGS__)
#define TBM_EMULATOR_LOG_ERROR(format, ...) tbm_emulator_log_event(tbm_emulator_log_level_error, __FUNCTION__, __LINE__, format,##__VA_ARGS__)

#endif
