#pragma once

#include <cassert>
#include <cstdint>
#include <limits>

namespace rex::graphics {

// Entries within each shape are ordered by retirement sequence. The GPU worker
// owns the pool; only resources already eligible for recycling are removed.
template <typename Pool>
bool EvictOldestReusableEntry(Pool& pool, uint64_t& pooled_bytes) {
  auto oldest = pool.end();
  uint64_t oldest_sequence = std::numeric_limits<uint64_t>::max();
  for (auto it = pool.begin(); it != pool.end(); ++it) {
    if (!it->second.empty() && it->second.front().retired_sequence < oldest_sequence) {
      oldest = it;
      oldest_sequence = it->second.front().retired_sequence;
    }
  }
  if (oldest == pool.end()) return false;
  assert(pooled_bytes >= oldest->second.front().size);
  pooled_bytes -= oldest->second.front().size;
  oldest->second.pop_front();
  if (oldest->second.empty()) pool.erase(oldest);
  return true;
}

}  // namespace rex::graphics
