#include <rex/system/mmio_handler.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <sys/mman.h>
#include <unistd.h>
struct State {
  void* base;
  size_t page;
  bool watched = false;
  unsigned handled = 0, fallback = 0, attempts = 0;
};
static void Check(bool value) { if (!value) std::abort(); }
int main() {
  State state{};
  state.page = size_t(sysconf(_SC_PAGESIZE));
  state.base = mmap(nullptr, state.page * 2, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  Check(state.base != MAP_FAILED);
  auto* bytes = static_cast<uint8_t*>(state.base);
  auto handler = rex::runtime::MMIOHandler::Install(
      bytes, bytes + state.page, bytes + state.page * 2 - 1,
      [](const void* context, const void* address) -> uint32_t {
        const auto* state = static_cast<const State*>(context);
        return uint32_t(static_cast<const uint8_t*>(address) - static_cast<const uint8_t*>(state->base));
      }, &state,
      [](rex::thread::global_unique_lock_type lock, void* context, void*, bool) {
        auto* state = static_cast<State*>(context);
        ++state->fallback;
        return mprotect(state->base, state->page, PROT_READ | PROT_WRITE) == 0;
      }, &state);
  Check(bool(handler));
  handler->SetTrackedWriteCallback(
      [](rex::thread::global_unique_lock_type lock, void* context, void*, bool write) {
        auto* state = static_cast<State*>(context);
        ++state->attempts;
        if (!write || !state->watched) return false;
        state->watched = false; ++state->handled;
        return mprotect(state->base, state->page, PROT_READ | PROT_WRITE) == 0;
      });
  state.watched = true;
  Check(mprotect(state.base, state.page, PROT_READ) == 0);
  std::atomic<bool> ready{false};
  std::vector<std::thread> workers;
  auto* words = static_cast<volatile uint32_t*>(state.base);
  for (unsigned i = 0; i < 8; ++i) workers.emplace_back([&, i] {
    while (!ready.load(std::memory_order_acquire)) std::this_thread::yield();
    words[i] = i + 1;
  });
  ready.store(true, std::memory_order_release);
  for (auto& worker : workers) worker.join();
  Check(state.handled == 1 && state.fallback == 0);
  for (unsigned i = 0; i < 8; ++i) Check(words[i] == i + 1);
  Check(mprotect(state.base, state.page, PROT_READ) == 0);
  words[0] = 99;
  Check(words[0] == 99 && state.fallback == 1);
  const unsigned attempts = state.attempts;
  Check(mprotect(state.base, state.page, PROT_NONE) == 0);
  const uint32_t read_value = words[0];
  Check(read_value == 99 && state.fallback == 2 && state.attempts == attempts);
  handler.reset();
  Check(munmap(state.base, state.page * 2) == 0);
  std::puts("PASS: concurrent watched writes, stale-fault race, untracked write and read fallback");
}
