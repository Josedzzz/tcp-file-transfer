#include "logger.h"
#include <stdlib.h>

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
}
