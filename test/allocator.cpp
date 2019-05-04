#include <pink/allocator.hpp>
#include <pink/box.hpp>

#include <iostream>

using namespace pink::prelude;

// invalidly implemented, but good enough to test allocators
struct MyMR final : pink::MemoryResource {
  void* do_allocate(pink::Int size, pink::Int) const noexcept override {
    return ::operator new(size);
  }
  void do_free(void*, pink::Int, pink::Int) const noexcept override {}
};
const MyMR my_mr;

// mostly exists to make sure tests work
int main() {
  auto b = Box<Box<int>, PolymorphicAllocator>(
    AllocatorArgument(), PolymorphicAllocator(&my_mr), 0);

  static_assert(pink::same_type<
                decltype(b)::ValueType,
                Box<int, PolymorphicAllocator>>);

  auto b2 = pink::
    Box<pink::Box<int, SystemAllocator>, pink::PolymorphicAllocator>(
      AllocatorArgument(), PolymorphicAllocator(&my_mr), 0);

  static_assert(
    pink::
      same_type<decltype(b2)::ValueType, Box<int, SystemAllocator>>);

  if (b->allocator() != pink::PolymorphicAllocator(&my_mr)) {
    return 1;
  }

  if (**b != 0) {
    return 1;
  }

  if (**b2 != 0) {
    return 1;
  }
}
