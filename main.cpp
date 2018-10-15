#include <Splash/Allocators.hpp>
#include <Splash/Barriers.hpp>

#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cstdint>

constexpr std::size_t MAX_CHILD_SIZE =      1000;
constexpr std::size_t ROOT_SIZE      =      1000;
constexpr std::size_t ITERATIONS     = 100000000;
constexpr std::size_t SLOT_STRIDE    =         3;

constexpr std::size_t childSize(std::size_t i) {
  return i % MAX_CHILD_SIZE;
}

constexpr std::size_t index(std::size_t i) {
  return (i * SLOT_STRIDE) % ROOT_SIZE;
}

__attribute__((noinline))
void gc_bench(OMR::GC::RunContext& cx) {
  /// SPLASH TODO
}

__attribute__((noinline))
void malloc_bench() {
  void** root = reinterpret_cast<void**>(std::calloc(ROOT_SIZE, sizeof(void*)));
  for (std::size_t i = 0; i < ITERATIONS; i++) {
    std::size_t idx = index(i);
    void* child = std::malloc(childSize(i));
    if (root[idx]) {
      std::free(root[idx]);
    }
    root[idx] = child;
  }
}

/// Call f(args), and returns the wallclock duration in seconds.
template <typename F, typename... Args>
double
time(F&& f, Args&&... args)
{
  auto start = std::chrono::high_resolution_clock::now();
  f(std::forward<Args>(args)...);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double>  duration = end - start;
  return duration.count();
}

/// Call f(args) n times, and return the average wallclock duration in seconds
template <typename F, typename... Args>
double
averageTime(const std::size_t n, F&& f, Args&&... args)
{
  if (n == 0) {
    throw(std::invalid_argument("n must be greater than zero"));
  }

  double sum = 0;

  for(std::size_t i = 0; i < n; ++i) {
    sum += time(f, std::forward<Args>(args)...);
  }

  return sum / n;

}

extern "C" int
main(int argc, char** argv)
{
  std::size_t n = 1;

  OMR::Runtime runtime;
  OMR::GC::System system(runtime);
  OMR::GC::RunContext context(system);

  double gc_time     = averageTime(n, gc_bench, context);
  double malloc_time = averageTime(n, malloc_bench);
  double diff = malloc_time - gc_time;

  std::cout
    << "GC:     " << gc_time     << " seconds" << "\n"
    << "malloc: " << malloc_time << " seconds" << "\n"
    << "diff:   " << diff        << " seconds" << std::endl;
  
  return 0;
}