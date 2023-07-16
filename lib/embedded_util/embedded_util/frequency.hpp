#pragma once

#include <chrono>

/// Type-safe containers for frequencies
///
/// std::chrono provides similar features for durations so the logic can be reused
namespace frequency {

// Since we are mapping frequencies to durations, the units are inverted (ex. 1 GHz should be
// multiplied to 1000 MHz as 1 second is multiplied to 1000 ms)

using GHz = std::chrono::duration<uint64_t>;
using MHz = std::chrono::duration<uint64_t, std::milli>;
using KHz = std::chrono::duration<uint64_t, std::micro>;
using Hz = std::chrono::duration<uint64_t, std::nano>;

} // namespace frequency

using Frequency = frequency::Hz;
