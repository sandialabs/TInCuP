project('tincup', 'cpp',
  version : '0.1.0',
  license : 'MIT', # You should add a LICENSE file with the license text
  default_options : ['cpp_std=c++20'])

# Reference the header within this subproject's directory structure
# When used as a Meson subproject, this must stay within the sandbox

# This creates a dependency object that other Meson projects can use
# via `dependency('tincup')` when this project is used as a subproject.
tincup_inc = include_directories('include')
tincup_dep = declare_dependency(
  include_directories : tincup_inc,
  version : meson.project_version()
)

# This makes the dependency object available to parent projects.
set_variable('tincup_dep', tincup_dep)

# Also make it available as a proper meson dependency
meson.override_dependency('tincup', tincup_dep)

# --- Installation ---

# Install the C++ headers for system-wide availability
install_headers(
  'include/tincup/tincup.hpp',
  subdir : 'tincup'
)

# Note: Vim plugin installation requires access to files outside the subproject
# and is handled by the root meson.build

# A helpful message for the user
message('C++ headers and Vim plugin can be installed with `meson install`.')
message('The Python-based code generator should be installed separately via `pip install .`')

# --- Examples ---
# Examples are handled by the root meson.build since they need to access 
# files outside this subproject directory

# --- Tests ---
# Only build tests if this is the main project (not a subproject)
if not meson.is_subproject()
  test_src = files('tests/test_static_dispatch.cpp')
  test_exe = executable('test_static_dispatch', test_src,
                        include_directories: [tincup_inc])
  test('static_dispatch', test_exe)
endif
