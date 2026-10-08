#include <rex/graphics/shared_memory.h>
#include <rex/system/xmemory.h>
#include <rex/cvar.h>
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
REXCVAR_DEFINE_BOOL(gpu_command_stats, false, "Test", "Disabled test diagnostics");
static void Check(bool ok) { if (!ok) std::abort(); }
class Mirror final : public rex::graphics::SharedMemory {
 public:
  Mirror(rex::memory::Memory& memory, uint32_t base, uint32_t size)
      : SharedMemory(memory), base_(base), data(size) { InitializeCommon(); }
  unsigned copies = 0;
  std::vector<uint8_t> data;
 private:
  uint32_t base_;
  bool UploadRanges(const std::vector<std::pair<uint32_t,uint32_t>>& ranges) override {
    for (auto [page, count] : ranges) {
      uint32_t start = page << page_size_log2(), size = count << page_size_log2();
      Check(start >= base_ && start - base_ + size <= data.size());
      MakeRangeValid(start, size, false);
      std::memcpy(data.data() + start - base_, memory().TranslatePhysical(start), size);
      ++copies;
    }
    return true;
  }
};
int main() {
  rex::memory::Memory memory;
  Check(memory.Initialize());
  constexpr uint32_t size = 65536;
  constexpr uint32_t rw = rex::memory::kMemoryProtectRead | rex::memory::kMemoryProtectWrite;
  for (uint32_t alias : {0xA0000000u, 0xC0000000u, 0xE0000000u}) {
    auto* heap = static_cast<rex::memory::PhysicalHeap*>(memory.LookupHeap(alias));
    uint32_t address = 0;
    Check(heap->Alloc(size, size, rex::memory::kMemoryAllocationReserve |
                                 rex::memory::kMemoryAllocationCommit, rw, false, &address));
    auto* guest = memory.TranslateVirtual<uint8_t*>(address);
    std::memset(guest, 0, size);
    {
      Mirror mirror(memory, heap->GetPhysicalAddress(address), size);
      uint32_t physical = heap->GetPhysicalAddress(address);
      Check(mirror.RequestRange(physical, size));
      for (unsigned round = 0; round < 16; ++round) {
        unsigned before = mirror.copies;
        for (unsigned request = 0; request < 8; ++request) Check(mirror.RequestRange(physical, size));
        Check(mirror.copies == before); // Simulate frames without a blanket cache flush.
        std::atomic<bool> ready{false};
        std::vector<std::thread> workers;
        for (unsigned i = 0; i < 8; ++i) workers.emplace_back([&, i] {
          while (!ready.load(std::memory_order_acquire)) std::this_thread::yield();
          guest[i * 4096] = uint8_t(round * 8 + i + 1);
        });
        ready.store(true, std::memory_order_release);
        for (auto& worker : workers) worker.join();
        Check(mirror.RequestRange(physical, size));
        Check(mirror.copies > before);
        Check(std::memcmp(mirror.data.data(), guest, size) == 0);
      }
      unsigned before = mirror.copies;
      mirror.RangeWrittenByGpu(physical, 4096);
      Check(mirror.RequestRange(physical, 4096));
      Check(mirror.copies == before); // Preserve GPU-owned data until a CPU write.
      guest[0] = 231;
      Check(mirror.RequestRange(physical, 4096));
      Check(mirror.copies > before && mirror.data[0] == 231);
    }
    Check(heap->Release(address));
  }
  std::puts("PASS: retained shared-memory cache, 48 concurrent-write rounds across A/C/E aliases, unchanged reuse, GPU ownership and CPU invalidation");
}
