project(
  'pink',
  'cpp',
  version: '0.1.0',
  license: 'Apache-2.0',
  default_options: [
    'werror=true',
    'warning_level=3',
    'cpp_std=c++17',
    'default_library=static'])

build_defines = ['-DBUILDING_PINK']
includes = include_directories('include')

library = library(
  'pink',
  'src/allocator.cpp',
  'src/base.cpp',
  include_directories: includes,
  cpp_args: build_defines,
  install: true,
)

pink_dep = declare_dependency(
  link_with: library,
  include_directories: includes,
)

install_subdir('include/pink', install_dir : 'include')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : 'pink',
  filebase : 'pink',
  description : 'Nicole\'s Pink non-standard standard library',
  subdirs : 'pink',
  libraries : library,
  version : '0.1',
)

subdir('test')
