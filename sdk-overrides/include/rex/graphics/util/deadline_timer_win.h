#pragma once

#include <cstdint>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace rex::graphics {

// Single-owner timer for the GPU vblank worker. If unavailable, callers retain
// their ordinary sleep path. No spinning or change to guest tick cadence.
class DeadlineTimerWin {
 public:
  DeadlineTimerWin()
      : handle_(CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                      TIMER_ALL_ACCESS)) {}
  ~DeadlineTimerWin() { if (handle_) CloseHandle(handle_); }
  DeadlineTimerWin(const DeadlineTimerWin&) = delete;
  DeadlineTimerWin& operator=(const DeadlineTimerWin&) = delete;

  bool Wait(int64_t duration_100ns) const {
    if (!handle_ || duration_100ns <= 0) return false;
    LARGE_INTEGER due_time;
    due_time.QuadPart = -duration_100ns;
    return SetWaitableTimer(handle_, &due_time, 0, nullptr, nullptr, FALSE) &&
           WaitForSingleObject(handle_, 50) == WAIT_OBJECT_0;
  }

 private:
  HANDLE handle_;
};

}  // namespace rex::graphics
