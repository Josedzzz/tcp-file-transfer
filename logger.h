#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

typedef enum {
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR
} log_level_t;

typedef struct {
  FILE *file;
  log_level_t min_level;
  int use_stdout;
  int use_colors;
} logger_t;

// Initialize
logger_t *logger_create(const char *filename, log_level_t level);
void logger_destroy(logger_t *logger);

void logger_log(logger_t *logger, log_level_t level, const char *file, int line,
                const char *func, const char *format, ...);

// Macros
#define LOG_DEBUG(logger, ...)                                                 \
  logger_log(logger, LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(logger, ...)                                                  \
  logger_log(logger, LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(logger, ...)                                                  \
  logger_log(logger, LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(logger, ...)                                                 \
  logger_log(logger, LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

#endif // !LOGGER_H
