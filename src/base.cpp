#include <pink/base.hpp>

#include <cstdlib>
#include <iostream>

namespace pink {

[[noreturn]] void
_Fail(char const* msg, char const* func, int line, char const* file) {
  std::cerr << msg << " in " << func << '\n';
  std::cerr << "  at " << file << ':' << line << '\n';
  std::abort();
}

} // namespace pink
