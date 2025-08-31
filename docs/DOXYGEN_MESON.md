# Doxygen with Meson

This guide shows how to integrate Doxygen generation into a Meson build, similar to
using a `Doxyfile.in` with CMake.

## 1) Create `Doxyfile.in`

Place this template at your project root (or under `docs/`). Meson will substitute
the `@VAR@` placeholders.

```
# --- Project ---
PROJECT_NAME           = @PROJECT_NAME@
PROJECT_NUMBER         = @PROJECT_VERSION@
OUTPUT_DIRECTORY       = @DOXY_OUTPUT_DIR@
USE_MDFILE_AS_MAINPAGE = @DOXY_MAINPAGE@

# --- Inputs ---
INPUT                  = @DOXY_INPUT@
FILE_PATTERNS          = *.hpp *.h *.hh *.cpp *.cc *.cxx *.md *.dox
RECURSIVE              = YES
EXTRACT_ALL            = NO

# --- Aliases for TInCuP tags ---
ALIASES               += cpo="Customization Point Object"
ALIASES               += cpo_example="Example usage of CPO"
ALIASES               += tag_invoke_impl="Default tag_invoke overload implementation"
ALIASES               += cpo_impl="CPO trait specialization (formatter-style)"
ALIASES               += cpo_adl_shim="ADL shim forwarding to cpo_impl"

# --- Groups ---
ENABLE_PREPROCESSING   = YES
ENABLE_SERVER_SIDE     = NO
EXTRACT_STATIC         = YES
GENERATE_TREEVIEW      = YES
GENERATE_LATEX         = NO
GENERATE_HTML          = YES
HTML_OUTPUT            = html

# Ensure your Doxyfile INPUT includes docs/DOXYGEN_GROUPS.dox
@DOXY_GROUPS@
```

Recommended values for placeholders:
- `@DOXY_INPUT@`: include paths to your public headers and `docs/DOXYGEN_GROUPS.dox`.
- `@DOXY_MAINPAGE@`: a Markdown file like `README.md` or leave blank.
- `@DOXY_OUTPUT_DIR@`: a build dir, e.g. `@BUILD_ROOT@/doc` (Meson fills this).

## 2) Meson build logic

Add an option to toggle docs, configure the Doxyfile, and define a target.

```meson
# meson.options
option('docs', type: 'boolean', value: false, description: 'Build API docs with Doxygen')
```

```meson
# meson.build (root)
doxygen = find_program('doxygen', required: get_option('docs'))

if doxygen.found() and get_option('docs')
  conf = configuration_data()
  conf.set('PROJECT_NAME', meson.project_name())
  conf.set('PROJECT_VERSION', meson.project_version())

  # Inputs: public headers and Doxygen groups file
  doxy_inputs = [
    join_paths(meson.project_source_root(), 'include'),
    join_paths(meson.project_source_root(), 'docs', 'DOXYGEN_GROUPS.dox'),
  ]
  conf.set_quoted('DOXY_INPUT', ' '.join(doxy_inputs))

  # Main page (optional)
  conf.set_quoted('DOXY_MAINPAGE', join_paths(meson.project_source_root(), 'README.md'))

  # Output directory in the build tree
  outdir = join_paths(meson.project_build_root(), 'doc')
  conf.set_quoted('DOXY_OUTPUT_DIR', outdir)

  # Ensure groups file is included even if INPUT is overridden elsewhere
  conf.set('DOXY_GROUPS', 'INPUT            += ' + join_paths(meson.project_source_root(), 'docs', 'DOXYGEN_GROUPS.dox'))

  doxyfile = configure_file(
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    configuration: conf,
  )

  doc_target = custom_target('doxygen-docs',
    input: doxyfile,
    output: 'html',
    command: [doxygen, '@INPUT@'],
    build_by_default: true,
  )

  # Optional install of generated HTML
  install_subdir(
    join_paths('doc', 'html'),
    install_dir: join_paths(get_option('datadir'), 'doc', meson.project_name()),
    strip_directory: false,
  )
endif
```

Notes:
- If using TInCuP as a Meson subproject, generate docs in the consuming project’s
  root so the Doxygen `INPUT` can reference both your headers and TInCuP’s headers.
- On CI, you can enable docs with `-Ddocs=true` and upload `build/doc/html`.

## 3) Running

```bash
meson setup build -Ddocs=true
meson compile -C build
# Open build/doc/html/index.html
```

This approach mirrors the CMake Doxyfile.in workflow while staying idiomatic for Meson.

