# Scripts

This directory contains utility scripts for maintaining the TInCuP project.

## `update_readme_examples.py`

**JSON-driven approach** that automatically regenerates the entire Examples section in README.md from a configuration file. This is the modern, maintainable approach.

### Configuration File: `readme_examples.json`

Examples are defined declaratively in JSON format:

```json
{
  "examples": [
    {
      "name": "Generic CPO",
      "description": "A basic CPO with generic template parameters",
      "command_args": ["{\"cpo_name\": \"generic_cpo\", \"args\": [\"$T1&: arg1\", \"$T2&: arg2\"]}"],
      "order": 1
    }
  ]
}
```

### Features

- **JSON Configuration**: All examples defined in `readme_examples.json`
- **Complete Regeneration**: Replaces entire Examples section from scratch
- **No Pattern Matching**: Eliminates regex complexity and edge cases
- **Descriptions**: Rich descriptions for each example type
- **Usage Examples**: Optional usage code blocks for dispatch CPOs
- **Ordered Output**: Examples appear in specified order
- **Error Handling**: Clear error messages with proper validation

### Usage

```bash
# Update README.md with latest generator output
python3 scripts/update_readme_examples.py

# Preview what would be updated without making changes
python3 scripts/update_readme_examples.py --dry-run

# Use custom configuration file
python3 scripts/update_readme_examples.py --config custom.json

# Via Make targets (convenient aliases)
make update-readme-examples
make preview-readme-examples
```

### Expected Format

The script looks for collapsible sections in this format:

```markdown
<details>
<summary><strong>Section Name</strong></summary>

**Command:**
```bash
cpo-generator '{"cpo_name": "example", "args": ["$T&: arg"]}'
```

**Generated Code:**
```cpp
// Generated CPO code here
```

</details>
```

### Adding New Examples

To add a new example, simply edit `readme_examples.json`:

```json
{
  "name": "My New CPO",
  "description": "Description of what this CPO demonstrates",
  "command_args": ["{\"cpo_name\": \"my_cpo\", \"args\": [\"$T&: arg\"]}"],
  "usage_examples": ["// Optional usage examples"],
  "order": 7
}
```

### Integration with Development Workflow

This script should be run whenever:

1. **Template Changes**: After modifying Jinja2 templates in `cpo_tools/templates/`
2. **Generator Updates**: After changes to `cpo_generator.py` logic
3. **New Examples**: After adding examples to `readme_examples.json`
4. **Before Releases**: To ensure documentation accuracy
5. **CI/CD Integration**: Could be added as a check to ensure docs are up-to-date

### Technical Details

- **JSON Configuration**: Clean, declarative example definitions
- **Section Replacement**: Replaces entire Examples section atomically
- **Command Execution**: Direct subprocess calls to cpo_generator
- **Error Recovery**: Clear error messages for each example
- **Cross-platform**: Works on Linux, macOS, and Windows

### Example Output

```
Processing README: README.md
Using configuration: scripts/readme_examples.json
Loaded 6 examples from configuration
Found Examples section at positions 17890-29696

Generating new Examples section...
Generating example: Generic CPO
Generating example: Concrete CPO
Generating example: Boolean Dispatch CPO
Generating example: String Dispatch CPO

✓ Updated Examples section in README.md
```

## Files in this Directory

- **`update_readme_examples.py`** - Main JSON-driven script
- **`readme_examples.json`** - Configuration file defining all examples  
- **`README.md`** - This documentation file

This system ensures that the TInCuP documentation remains accurate and current as the code generation system evolves.
