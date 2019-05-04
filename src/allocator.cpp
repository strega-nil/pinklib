#include <pink/allocator.hpp>

#include <new>

namespace pink {

SystemMemoryResource const system_memory_resource;

void* SystemMemoryResource::do_allocate(Int size, Int align) const
  noexcept {
  return ::operator new(size, std::align_val_t(align), std::nothrow);
}
void SystemMemoryResource::do_free(void* ptr, Int size, Int align) const
  noexcept {
  ::operator delete(ptr, size, std::align_val_t(align));
}

} // namespace pink
