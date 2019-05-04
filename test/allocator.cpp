#include <pink/allocator.hpp>
#include <pink/box.hpp>

#include <iostream>

// invalidly implemented, but good enough to test allocators
struct MyMR : pink::MemoryResource {
  void* do_allocate(pink::Int size, pink::Int) const noexcept override {
    return ::operator new(size);
  }
  void do_free(void*, pink::Int, pink::Int) const noexcept override {}
};
const MyMR my_mr;

// mostly exists to make sure tests work
int main() {
  auto b = pink::Box<pink::Box<int>, pink::PolymorphicAllocator>(
    pink::AllocatorArgument(), pink::PolymorphicAllocator(&my_mr), 0);

  static_assert(pink::same_type<
                decltype(b)::ValueType,
                pink::Box<int, pink::PolymorphicAllocator>>);

  auto b2 = pink::Box<
    pink::Box<int, pink::SystemAllocator>,
    pink::PolymorphicAllocator
  >(
    pink::AllocatorArgument(), pink::PolymorphicAllocator(&my_mr), 0);

  static_assert(pink::same_type<
                decltype(b2)::ValueType,
                pink::Box<int, pink::SystemAllocator>>);

  if (b->allocator() == pink::PolymorphicAllocator(&my_mr)) {
    return 0;
  } else {
    return 1;
  }
}
