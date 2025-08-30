# Meson Integration

Use this directory to integrate `tincup` with Meson-based projects.

## Quick Start

```bash
# From your project root
cd build_systems/meson
meson setup builddir
meson compile -C builddir
```

## Integration with Your Project

Add to your `meson.build`:

```meson
# Using subproject (recommended)
tincup_proj = subproject('tincup', 
                         default_options: ['cpp_std=c++20'])
tincup_dep = tincup_proj.get_variable('tincup_dep')

executable('my_exe', 'main.cpp',
  dependencies : [tincup_dep])
```

Or using dependency fallback:

```meson
tincup_dep = dependency('tincup', 
                        fallback : ['tincup', 'tincup_dep'],
                        default_options: ['cpp_std=c++20'])
```

## What's Included

- Dependency: `tincup_dep`
- Header-only library with C++20 requirement
- Include path handling
- Meson best practices compliance

## Important Notes

- This directory contains a local copy of the headers (`include/tincup/`) to avoid Meson subproject sandbox violations
- The headers are kept in sync manually with the main `include/` directory
- When the main header is updated, remember to update the copy: `cp ../../include/tincup/tincup.hpp include/tincup/`