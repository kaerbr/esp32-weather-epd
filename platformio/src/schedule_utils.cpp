#include "schedule_utils.h"
#include "config.h"

// Helper: Parses a time string in "HH:MM" format and returns minutes since midnight.
int parseTimeStr(const char *timeStr) {
  int hour = 0, minute = 0;
  sscanf(timeStr, "%d:%d", &hour, &minute);
  return hour * 60 + minute;
}

// Computes and returns the next scheduled update time (as a time_t) based on the current time.
time_t getNextUpdateTime(time_t currentTime) {
  struct tm currentTM;
  localtime_r(&currentTime, &currentTM);
  int currentDay = currentTM.tm_wday;  // 0 = Sunday, 1 = Monday, etc.
  int currentMinutes = currentTM.tm_hour * 60 + currentTM.tm_min;
  time_t bestCandidate = 0;  // Will hold the earliest candidate time for today

  // Loop through each schedule segment that applies today.
  for (int i = 0; i < scheduleSegmentsCount; i++) {
    // Skip segments that don't apply today.
    if (!(scheduleSegments[i].dayMask & (1 << currentDay))) {
      continue;
    }

    // Convert start and end times from HH:MM to minutes since midnight.
    int segStart = parseTimeStr(scheduleSegments[i].startTime);
    int segEnd = parseTimeStr(scheduleSegments[i].endTime);
    int candidateMinutes = -1;

    // Case 1: If the current time is before the segment starts, the candidate is the segment's start.
    if (currentMinutes < segStart) {
      candidateMinutes = segStart;
    }
    // Case 2: We are within the segment.
    else {
      // Use '<' so that if currentMinutes equals segEnd, we don't schedule an update today.
      if (currentMinutes < segEnd) {
        int elapsed = currentMinutes - segStart;
        int remainder = elapsed % scheduleSegments[i].interval;
        // **Bug Fix:** If remainder is 0, we want the next update, so use the full interval.
        int offset = (remainder == 0) ? scheduleSegments[i].interval : (scheduleSegments[i].interval - remainder);
        candidateMinutes = currentMinutes + offset;
        // If candidate exceeds the segment's end, invalidate it.
        if (candidateMinutes > segEnd) {
          candidateMinutes = -1;
        }
      }
    }

    // If a valid candidate was found, build its time_t.
    if (candidateMinutes != -1) {
      struct tm candidateTM = currentTM;
      candidateTM.tm_hour = candidateMinutes / 60;
      candidateTM.tm_min = candidateMinutes % 60;
      candidateTM.tm_sec = 0;
      time_t candidateTime = mktime(&candidateTM);

      // Safety: If candidate time is in the past (which shouldn't happen), add one day.
      if (candidateTime + 60 < currentTime) {
        candidateTime += 24 * 60 * 60;
      }

      // Select the earliest candidate.
      if (bestCandidate == 0 || candidateTime < bestCandidate) {
        bestCandidate = candidateTime;
      }
    }
  }

  // If no candidate was found for today, search the next days (up to 7 days ahead).
  if (bestCandidate == 0) {
    for (int offset = 1; offset <= 7; offset++) {
      // Calculate time for 'offset' days in the future.
      time_t candidateDayTime = currentTime + offset * 24 * 60 * 60;
      struct tm candidateTM;
      localtime_r(&candidateDayTime, &candidateTM);
      int day = candidateTM.tm_wday;

      // Find the earliest start time for this day.
      int earliest = 24 * 60;  // Maximum possible minutes in a day.
      for (int i = 0; i < scheduleSegmentsCount; i++) {
        if (scheduleSegments[i].dayMask & (1 << day)) {
          int segStart = parseTimeStr(scheduleSegments[i].startTime);
          if (segStart < earliest) {
            earliest = segStart;
          }
        }
      }

      // If we found a valid start time, update candidateTM and break.
      if (earliest < 24 * 60) {
        candidateTM.tm_hour = earliest / 60;
        candidateTM.tm_min = earliest % 60;
        candidateTM.tm_sec = 0;
        bestCandidate = mktime(&candidateTM);
        break;
      }
    }
  }

  // Ensure the returned time is strictly in the future.
  if (bestCandidate <= currentTime) {
    bestCandidate = currentTime + 1;
  }

  return bestCandidate;
}

// Returns the sleep duration (in seconds) until the next scheduled update.
uint64_t getSleepDurationSeconds(time_t now) {
  time_t nextUpdate = getNextUpdateTime(now);
  return (uint64_t)(nextUpdate - now);
}
