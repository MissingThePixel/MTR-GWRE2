#include <rex/thread/fiber.h>
#include <cstdio>
#include <cstdlib>
using rex::thread::Fiber;
static Fiber* main_fiber;
static unsigned count;
static bool finished;
static uint64_t main_fpcr, main_fpsr;
static uint64_t ReadFpcr() { uint64_t v; asm volatile("mrs %0, fpcr" : "=r"(v)); return v; }
static uint64_t ReadFpsr() { uint64_t v; asm volatile("mrs %0, fpsr" : "=r"(v)); return v; }
static void Check(bool good) { if (!good) std::abort(); }
static void Worker(void*) {
  volatile unsigned stack_marker = 0x12345678;
  uint64_t fpcr = (main_fpcr & ~uint64_t(3 << 22)) | (1 << 22);
  uint64_t fpsr = 0x10;
  asm volatile("msr fpcr, %0" :: "r"(fpcr));
  asm volatile("msr fpsr, %0" :: "r"(fpsr));
  for (unsigned i = 0; i < 10000; ++i) {
    Check(count == i);
    ++count;
    Fiber::SwitchTo(main_fiber);
    Check(stack_marker == 0x12345678 && ReadFpcr() == fpcr && ReadFpsr() == fpsr);
  }
  finished = true;
  Fiber::SwitchTo(main_fiber);
  std::abort();
}
int main() {
  main_fpcr = ReadFpcr(); main_fpsr = ReadFpsr();
  main_fiber = Fiber::ConvertCurrentThread();
  Fiber* worker = Fiber::Create(65536, Worker, nullptr);
  volatile unsigned stack_marker = 0xABCDEF01;
  for (unsigned i = 0; i < 10000; ++i) {
    Fiber::SwitchTo(worker);
    Check(count == i + 1 && stack_marker == 0xABCDEF01);
    Check(ReadFpcr() == main_fpcr && ReadFpsr() == main_fpsr);
  }
  Fiber::SwitchTo(worker);
  Check(finished && Fiber::Current() == main_fiber);
  worker->Destroy(); main_fiber->Destroy();
  Check(Fiber::Current() == nullptr);
  std::puts("PASS: 10000 fiber round trips; stacks, FPCR/FPSR and ownership preserved");
}
