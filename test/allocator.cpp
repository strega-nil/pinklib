#include <pink/allocator.hpp>
#include <pink/box.hpp>

#include <iostream>

// mostly exists to make sure tests work
int main() {
  auto b = pink::Box<int, pink::SystemAllocator>(
      pink::AllocatorArgument(), pink::SystemAllocator(), 0);
}
