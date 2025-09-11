#!/usr/bin/env python3
# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

"""
README Examples Updater (JSON-driven approach)

⚠️  DEPRECATED: This script is deprecated since documentation restructuring.
    Use scripts/generate_examples_doc.py to generate docs/examples.md instead.

Legacy functionality: Automatically regenerates the Examples section in README.md from a JSON configuration file.
This replaces pattern matching with a clean, declarative approach where examples are
defined in JSON and the entire Examples section is regenerated from scratch.

Usage:
    python3 scripts/update_readme_examples.py [--dry-run] [--readme PATH] [--config PATH]
    
Recommended alternative:
    python3 scripts/generate_examples_doc.py [--dry-run] [--config PATH] [--output PATH]
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import List, Dict, Any
import re


def load_examples_config(config_path: Path) -> Dict[str, Any]:
    """Load examples configuration from JSON file."""
    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"ERROR: Configuration file not found: {config_path}")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"ERROR: Invalid JSON in {config_path}: {e}")
        sys.exit(1)


def run_cpo_generator(command_args: List[str]) -> str:
    """Run the CPO generator with the given arguments and return the output."""
    try:
        full_cmd = ['python3', '-m', 'cpo_tools.cpo_generator'] + command_args
        
        result = subprocess.run(
            full_cmd,
            capture_output=True,
            text=True,
            cwd=Path(__file__).parent.parent  # Run from project root
        )
        
        if result.returncode != 0:
            raise RuntimeError(f"Command failed: {' '.join(full_cmd)}\nStderr: {result.stderr}")
        
        return result.stdout.strip()
    
    except Exception as e:
        raise RuntimeError(f"Failed to run cpo-generator: {e}")


def generate_example_section(example: Dict[str, Any]) -> str:
    """Generate a collapsible markdown section for a single example."""
    name = example['name']
    description = example.get('description', '')
    command_args = example['command_args']
    usage_examples = example.get('usage_examples', [])
    
    # Generate the command string for display
    command_display = 'cpo-generator ' + ' '.join(command_args)
    
    # Generate the actual CPO code
    try:
        generated_code = run_cpo_generator(command_args)
    except Exception as e:
        print(f"WARNING: Failed to generate code for '{name}': {e}")
        generated_code = f"// Error generating code: {e}"
    
    # Build the section
    section = f"""<details>
<summary><strong>{name}</strong></summary>

{description}

**Command:**
```bash
{command_display}
```

**Generated Code:**
```cpp
{generated_code}
```"""

    # Add usage examples if provided
    if usage_examples:
        usage_code = '\n'.join(usage_examples)
        section += f"""

**Usage:**
```cpp
{usage_code}
```"""
    
    section += "\n\n</details>"
    
    return section


def generate_examples_section(config: Dict[str, Any]) -> str:
    """Generate the complete Examples section from the configuration."""
    template = config.get('template', {})
    header = template.get('section_header', '### Examples\n\n')
    footer = template.get('section_footer', '')
    
    examples = sorted(config['examples'], key=lambda x: x.get('order', 0))
    
    sections = [header]
    
    for example in examples:
        print(f"Generating example: {example['name']}")
        section = generate_example_section(example)
        sections.append(section)
        sections.append('')  # Add spacing between sections
    
    if footer:
        sections.append(footer)
    
    return '\n'.join(sections)


def find_examples_section(readme_content: str) -> tuple[int, int]:
    """
    Find the start and end positions of the Examples section in README content.
    Returns (start_pos, end_pos) or (-1, -1) if not found.
    """
    # Look for the Examples header
    examples_pattern = r'^### Examples\s*$'
    match = re.search(examples_pattern, readme_content, re.MULTILINE)
    
    if not match:
        print("WARNING: Could not find '### Examples' section in README")
        return -1, -1
    
    start_pos = match.start()
    
    # Find the next major section (## or ###) or end of file
    next_section_pattern = r'\n## [^#]|\n### [^E]|\Z'  # Avoid matching "### Examples" again
    next_match = re.search(next_section_pattern, readme_content[start_pos + 1:], re.MULTILINE)
    
    if next_match:
        end_pos = start_pos + 1 + next_match.start()
    else:
        end_pos = len(readme_content)
    
    return start_pos, end_pos


def update_readme_examples(readme_path: Path, config_path: Path, dry_run: bool = False) -> None:
    """Update the Examples section in the README file."""
    
    print(f"Processing README: {readme_path}")
    print(f"Using configuration: {config_path}")
    
    # Load configuration
    config = load_examples_config(config_path)
    print(f"Loaded {len(config['examples'])} examples from configuration")
    
    # Read current README
    try:
        readme_content = readme_path.read_text(encoding='utf-8')
    except FileNotFoundError:
        print(f"ERROR: README file not found: {readme_path}")
        sys.exit(1)
    
    # Find existing Examples section
    start_pos, end_pos = find_examples_section(readme_content)
    
    if start_pos == -1:
        print("ERROR: Could not find Examples section to replace")
        sys.exit(1)
    
    print(f"Found Examples section at positions {start_pos}-{end_pos}")
    
    # Generate new Examples section
    print("\nGenerating new Examples section...")
    new_examples_section = generate_examples_section(config)
    
    # Replace the section
    updated_content = (
        readme_content[:start_pos] + 
        new_examples_section + 
        readme_content[end_pos:]
    )
    
    # Write the updated content
    if not dry_run:
        try:
            readme_path.write_text(updated_content, encoding='utf-8')
            print(f"\n✓ Updated Examples section in {readme_path}")
        except Exception as e:
            print(f"\nERROR: Failed to write README: {e}")
            sys.exit(1)
    else:
        print(f"\n[DRY RUN] Would update Examples section in {readme_path}")
        print(f"New section would be {len(new_examples_section)} characters")


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Update CPO examples in README.md using JSON configuration",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 scripts/update_readme_examples.py                    # Update README.md
  python3 scripts/update_readme_examples.py --dry-run         # Preview changes
  python3 scripts/update_readme_examples.py --config custom.json  # Custom config
        """
    )
    
    parser.add_argument(
        '--readme',
        type=Path,
        default=Path('README.md'),
        help='Path to README file (default: README.md)'
    )
    
    parser.add_argument(
        '--config',
        type=Path,
        default=Path('scripts/readme_examples.json'),
        help='Path to examples configuration file (default: scripts/readme_examples.json)'
    )
    
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Show what would be updated without making changes'
    )
    
    return parser.parse_args()


def main():
    """Main entry point."""
    args = parse_args()
    
    # Validate paths
    if not args.readme.exists():
        print(f"ERROR: README file not found: {args.readme}")
        sys.exit(1)
    
    if not args.config.exists():
        print(f"ERROR: Configuration file not found: {args.config}")
        sys.exit(1)
    
    # Check if we can find the cpo_generator
    try:
        result = subprocess.run(['python3', '-m', 'cpo_tools.cpo_generator', '--help'], 
                              capture_output=True, cwd=Path(__file__).parent.parent)
        if result.returncode != 0:
            print("ERROR: Could not find cpo_tools.cpo_generator module")
            print("Make sure you're running from the project root and the module is installed")
            sys.exit(1)
    except Exception as e:
        print(f"ERROR: Could not test cpo_generator: {e}")
        sys.exit(1)
    
    # Update examples
    try:
        update_readme_examples(args.readme, args.config, args.dry_run)
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\nERROR: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()