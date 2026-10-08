#include <algorithm>
#include <chrono>
#include <thread>
#include <rex/cvar.h>
#include <rex/ppc/context.h>
#include <rex/ppc/func.h>

REXCVAR_DEFINE_INT32(android_gpu_poll_backoff_us, 0, "Android",
                    "Optional host backoff while the original guest GPU poll remains pending");

extern "C" REX_FUNC(__imp__sub_820A1B18);

// Keep the generated implementation, its return value, registers and guest
// clock intact. Back off only when it reports that the GPU wait is pending.
// Compiled only into the isolated Android target; disabled by default.
extern "C" REX_FUNC(sub_820A1B18) {
    const int delay = std::clamp(REXCVAR_GET(android_gpu_poll_backoff_us), 0, 500);
    __imp__sub_820A1B18(ctx, base);
    if (delay && ctx.r3.u32 != 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(delay));
    }
}