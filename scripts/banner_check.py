#!/usr/bin/env python3

# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

import os
import sys
import re
from pathlib import Path
from typing import List, Tuple, Optional
import fnmatch

# Default banner text embedded from BANNER.txt
BANNER_TEXT = (
    "TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`\n\n"
    "Copyright (c) National Technology & Engineering Solutions of Sandia, \n"
    "LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. \n"
    "Government retains certain rights in this software.\n\n"
    "Questions? Contact Greg von Winckel (gvonwin@sandia.gov)\n"
)

class BannerChecker:
    """
    This script recursively checks all C++ (.hpp, .cpp), Python (.py),
    CMakeLists.txt, and CMake (.cmake) files to ensure they contain
    the required copyright banner.
    """

    def __init__(self, banner_file: Optional[str] = None, banner_text: Optional[str] = None):
        self.banner_file = banner_file
        self.banner_text = self._load_banner(banner_file, banner_text)
        self.cpp_extensions = {'.hpp', '.cpp'}
        self.python_extensions = {'.py'}
        self.cmake_files = {'CMakeLists.txt'}
        self.cmake_extensions = {'.cmake'}
        self.gitignore_patterns = self._load_gitignore()
        
    def _load_banner(self, banner_file: Optional[str], banner_text: Optional[str]) -> str:
        """Resolve banner text from explicit value, file, or built-in default."""
        if banner_text:
            return banner_text.strip()
        if banner_file:
            try:
                with open(banner_file, 'r', encoding='utf-8') as f:
                    return f.read().strip()
            except FileNotFoundError:
                print(f"Warning: {banner_file} not found. Falling back to embedded banner text.")
            except Exception as e:
                print(f"Warning: Error reading {banner_file}: {e}. Falling back to embedded banner text.")
        return BANNER_TEXT.strip()
    
    def _load_gitignore(self) -> List[str]:
        """Load patterns from .gitignore file if it exists."""
        gitignore_path = Path(".gitignore")
        patterns = []
        if gitignore_path.exists():
            try:
                with open(gitignore_path, 'r', encoding='utf-8') as f:
                    for line in f:
                        line = line.strip()
                        if line and not line.startswith('#'):
                            patterns.append(line)
            except Exception as e:
                print(f"Warning: Error reading .gitignore: {e}")
        return patterns
    
    def _is_ignored(self, file_path: Path, root_dir: Path) -> bool:
        """Check if a file should be ignored based on .gitignore patterns."""
        # Get relative path from root directory
        try:
            relative_path = file_path.relative_to(root_dir)
        except ValueError:
            # Path is not relative to root_dir, don't ignore
            return False
        
        path_str = str(relative_path)
        path_parts = relative_path.parts
        
        for pattern in self.gitignore_patterns:
            # Handle directory patterns (ending with /)
            if pattern.endswith('/'):
                dir_pattern = pattern[:-1]
                if any(fnmatch.fnmatch(part, dir_pattern) for part in path_parts):
                    return True
            # Handle file patterns
            elif fnmatch.fnmatch(path_str, pattern) or fnmatch.fnmatch(file_path.name, pattern):
                return True
            # Handle patterns that match directory components
            elif any(fnmatch.fnmatch(part, pattern) for part in path_parts):
                return True
        
        return False
    
    def _get_cpp_banner_pattern(self) -> str:
        """Generate the expected C++ banner pattern"""
        escaped_banner = re.escape(self.banner_text)
        # Allow for whitespace variations in the comment block
        return rf'/\*\*\s*{escaped_banner}\s*\*/'
    
    def _commented_banner(self) -> str:
        """Return the banner commented with leading '#' per line (for Python/CMake)."""
        banner_lines = self.banner_text.split('\n')
        commented_lines = [f"# {line}".rstrip() if line.strip() else "#" for line in banner_lines]
        return "\n".join(commented_lines)
    
    def _check_cpp_file(self, file_path: Path) -> bool:
        """Check if a C++ file has the proper banner"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            pattern = self._get_cpp_banner_pattern()
            return bool(re.search(pattern, content, re.DOTALL))
        except Exception as e:
            print(f"Error reading {file_path}: {e}")
            return False
    
    def _check_python_file(self, file_path: Path) -> bool:
        """Check if a Python/CMake file has the proper banner"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # For Python files, skip shebang line if present and any immediate blank lines after it
            lines = content.split('\n')
            start_index = 0
            if file_path.suffix == '.py' and lines and lines[0].startswith('#!'):
                start_index = 1
                while start_index < len(lines) and lines[start_index].strip() == "":
                    start_index += 1
            
            # Check if banner appears at the start (after potential shebang)
            content_to_check = '\n'.join(lines[start_index:])
            commented_banner = self._commented_banner()
            return content_to_check.startswith(commented_banner)
        except Exception as e:
            print(f"Error reading {file_path}: {e}")
            return False
    
    def _should_check_file(self, file_path: Path) -> bool:
        """Determine if a file should be checked based on its name/extension"""
        if file_path.suffix in self.cpp_extensions:
            return True
        if file_path.suffix in self.python_extensions:
            return True
        if file_path.suffix in self.cmake_extensions:
            return True
        if file_path.name in self.cmake_files:
            return True
        return False
    
    def _get_file_type(self, file_path: Path) -> str:
        """Get the file type for reporting purposes"""
        if file_path.suffix in self.cpp_extensions:
            return "C++"
        elif file_path.suffix in self.python_extensions:
            return "Python"
        elif file_path.suffix in self.cmake_extensions or file_path.name in self.cmake_files:
            return "CMake"
        return "Unknown"
    
    def check_file(self, file_path: Path) -> bool:
        """Check a single file for banner compliance"""
        if file_path.suffix in self.cpp_extensions:
            return self._check_cpp_file(file_path)
        elif (file_path.suffix in self.python_extensions or 
              file_path.suffix in self.cmake_extensions or 
              file_path.name in self.cmake_files):
            return self._check_python_file(file_path)
        return True  # Skip files we don't check
    
    def scan_directory(self, root_dir: str = ".") -> Tuple[List[Path], List[Path]]:
        """Recursively scan directory and return lists of compliant and non-compliant files"""
        compliant_files = []
        non_compliant_files = []
        
        root_path = Path(root_dir)
        
        for file_path in root_path.rglob("*"):
            if (file_path.is_file() and 
                self._should_check_file(file_path) and 
                not self._is_ignored(file_path, root_path)):
                if self.check_file(file_path):
                    compliant_files.append(file_path)
                else:
                    non_compliant_files.append(file_path)
        
        return compliant_files, non_compliant_files

    # --- Fixers ---
    def _cpp_banner_block(self) -> str:
        return f"/**\n{self.banner_text}\n*/\n\n"

    def _python_cmake_banner_block(self) -> str:
        return self._commented_banner() + "\n\n"

    def fix_file(self, file_path: Path) -> bool:
        """Insert the appropriate banner into the file if missing. Returns True if modified."""
        if self.check_file(file_path):
            return False
        try:
            text = file_path.read_text(encoding='utf-8')
        except Exception as e:
            print(f"Error reading {file_path} for fixing: {e}")
            return False

        if file_path.suffix in self.cpp_extensions:
            new_text = self._cpp_banner_block() + text
        elif (file_path.suffix in self.python_extensions or
              file_path.suffix in self.cmake_extensions or
              file_path.name in self.cmake_files):
            # Respect Python shebang on first line
            if file_path.suffix == '.py' and text.startswith('#!'):
                lines = text.split('\n')
                shebang = lines[0]
                rest = '\n'.join(lines[1:])
                new_text = shebang + "\n" + self._python_cmake_banner_block() + rest
            else:
                new_text = self._python_cmake_banner_block() + text
        else:
            return False

        try:
            file_path.write_text(new_text, encoding='utf-8')
            return True
        except Exception as e:
            print(f"Error writing {file_path}: {e}")
            return False
    
    def generate_banner_examples(self):
        """Generate and print example banner formats for different file types"""
        print("Expected banner formats:")
        print("\nC++ files (.hpp, .cpp):")
        print(f"/**\n{self.banner_text}\n*/")
        
        print(f"\nPython (.py), CMake (.cmake), and CMakeLists.txt files:")
        banner_lines = self.banner_text.split('\n')
        for line in banner_lines:
            if line.strip():
                print(f"# {line}")
            else:
                print("#")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Check and optionally fix copyright banners in source files"
    )
    parser.add_argument(
        "--banner-file",
        default=None,
        help="Optional path to banner text file (overrides embedded banner)"
    )
    parser.add_argument(
        "--directory",
        default=".",
        help="Root directory to scan (default: current directory)"
    )
    parser.add_argument(
        "--fix",
        action="store_true",
        help="Automatically add missing banners to non-compliant files"
    )
    parser.add_argument(
        "--show-examples",
        action="store_true",
        help="Show expected banner format examples"
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Show all checked files, not just non-compliant ones"
    )
    
    args = parser.parse_args()
    
    checker = BannerChecker(args.banner_file)
    
    if args.show_examples:
        checker.generate_banner_examples()
        return 0
    
    
    print(f"Scanning {args.directory} for copyright banner compliance...")
    if args.banner_file:
        print(f"Using banner from: {args.banner_file}")
    else:
        print("Using embedded banner text")
    print()
    
    compliant_files, non_compliant_files = checker.scan_directory(args.directory)
    
    total_files = len(compliant_files) + len(non_compliant_files)

    if args.fix and non_compliant_files:
        print(f"Attempting to fix {len(non_compliant_files)} non-compliant files by inserting banners...\n")
        fixed = 0
        for file_path in sorted(non_compliant_files):
            if checker.fix_file(file_path):
                print(f"  + Fixed: {file_path}")
                fixed += 1
            else:
                print(f"  - Skipped: {file_path}")
        print(f"\nFixed {fixed}/{len(non_compliant_files)} files.")
        # Re-scan after fixing
        compliant_files, non_compliant_files = checker.scan_directory(args.directory)
        total_files = len(compliant_files) + len(non_compliant_files)
    
    if args.verbose:
        print("Compliant files:")
        for file_path in sorted(compliant_files):
            file_type = checker._get_file_type(file_path)
            print(f"  ✓ {file_path} ({file_type})")
        print()
    
    if non_compliant_files:
        print("Non-compliant files:")
        for file_path in sorted(non_compliant_files):
            file_type = checker._get_file_type(file_path)
            print(f"  ✗ {file_path} ({file_type})")
        print()
    
    print(f"Summary: {len(compliant_files)}/{total_files} files are compliant")
    
    if non_compliant_files:
        print(f"\n{len(non_compliant_files)} files need copyright banners added.")
        print("Run with --fix to add them automatically, or --show-examples to see the expected format.")
        return 1
    else:
        print("All files are compliant! ✓")
        return 0

if __name__ == "__main__":
    sys.exit(main())
