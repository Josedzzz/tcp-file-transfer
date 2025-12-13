#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ANSI color codes for better terminal output
#define COLOR_RESET "\x1b[0m"
#define COLOR_TRACE "\x1b[90m" // Black
#define COLOR_DEBUG "\x1b[36m" // Cyan
#define COLOR_INFO "\x1b[32m"  // Green
#define COLOR_WARN "\x1b[33m"  // Yellow
#define COLOR_ERROR "\x1b[31m" // Red
#define COLOR_FATAL "\x1b[35m" // Magenta

logger_t *logger_create(const char *filename, log_level_t level) {
  logger_t *logger = malloc(sizeof(logger_t));
  if (!logger)
    return NULL;

  logger->min_level = level;
  logger->use_stdout = 1;
  logger->use_colors = 1;

  if (filename) {
    logger->file = fopen(filename, "a");
    if (!logger->file) {
      free(logger);
      return NULL;
    }
    // Line buffering - writes happen at newline
    setvbuf(logger->file, NULL, _IOLBF, 0);
  } else {
    logger->file = stdout;
  }

  return logger;
}

void logger_log(logger_t *logger, log_level_t level, const char *file, int line,
                const char *func, const char *format, ...) {
  if (!logger || level < logger->min_level)
    return;

  // Get timestamp
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm);

  // Level string
  const char *level_str;
  const char *color = "";
  const char *reset = "";

  switch (level) {
  case LOG_LEVEL_DEBUG:
    level_str = "DEBUG";
    if (logger->use_colors)
      color = COLOR_DEBUG;
    break;
  case LOG_LEVEL_INFO:
    if (logger->use_colors)
      color = COLOR_INFO;
    break;
  case LOG_LEVEL_WARN:
  }
}
