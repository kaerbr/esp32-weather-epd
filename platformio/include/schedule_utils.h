#ifndef __SCHEDULE_UTILS_H__
#define __SCHEDULE_UTILS_H__

#include <time.h>

int parseTimeStr(const char* timeStr);
time_t getNextUpdateTime(time_t currentTime);
uint64_t getSleepDurationSeconds(time_t now);

#endif // __SCHEDULE_UTILS_H__
