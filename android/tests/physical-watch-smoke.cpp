#include <rex/system/xmemory.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
static void Check(bool value) { if (!value) std::abort(); }
int main() {
  rex::memory::Memory memory;
  Check(memory.Initialize());
  auto* heap = static_cast<rex::memory::PhysicalHeap*>(memory.LookupHeap(0xA0000000));
  uint32_t address = 0;
  constexpr uint32_t size = 65536;
  constexpr uint32_t rw = rex::memory::kMemoryProtectRead | rex::memory::kMemoryProtectWrite;
  Check(heap->Alloc(size, size, rex::memory::kMemoryAllocationReserve |
                               rex::memory::kMemoryAllocationCommit, rw, false, &address));
  const uint32_t physical = heap->GetPhysicalAddress(address);
  auto* words = memory.TranslateVirtual<volatile uint32_t*>(address);
  std::atomic<unsigned> invalidations{0};
  auto* callback = memory.RegisterPhysicalMemoryInvalidationCallback(
      [](void* context, uint32_t start, uint32_t length, bool) {
        static_cast<std::atomic<unsigned>*>(context)->fetch_add(1, std::memory_order_relaxed);
        return std::pair<uint32_t, uint32_t>{start, length};
      }, &invalidations);
  for (unsigned round = 0; round < 16; ++round) {
    memory.EnablePhysicalMemoryAccessCallbacks(physical, 4096, true, false);
    std::atomic<bool> ready{false};
    std::vector<std::thread> workers;
    for (unsigned i = 0; i < 8; ++i) workers.emplace_back([&, i] {
      while (!ready.load(std::memory_order_acquire)) std::this_thread::yield();
      words[i] = round * 8 + i + 1;
    });
    ready.store(true, std::memory_order_release);
    for (auto& worker : workers) worker.join();
    for (unsigned i = 0; i < 8; ++i) Check(words[i] == round * 8 + i + 1);
    Check(invalidations.load() == round + 1);
    Check(heap->HandleTrackedWriteFault(rex::thread::global_critical_region::AcquireDirect(), address));
  }
  Check(heap->Protect(address, size, rex::memory::kMemoryProtectRead));
  Check(!heap->HandleTrackedWriteFault(rex::thread::global_critical_region::AcquireDirect(), address));
  rex::memory::PageAccess access{}; size_t length = 0;
  Check(rex::memory::QueryProtect(memory.TranslateVirtual(address), length, access));
  Check(access == rex::memory::PageAccess::kReadOnly);
  Check(heap->Decommit(address, size));
  Check(!heap->HandleTrackedWriteFault(rex::thread::global_critical_region::AcquireDirect(), address));
  memory.UnregisterPhysicalMemoryInvalidationCallback(callback);
  Check(heap->Release(address));
  std::puts("PASS: 16 rounds of physical watch invalidation, concurrent writes, stale faults, read-only and decommitted guards");
}
