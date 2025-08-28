#!/usr/bin/env python3

# Real Vector Framework - Zero Overhead Abstractions for Vector Algorithms
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

class BannerChecker:
    """
    This script recursively checks all C++ (.hpp, .cpp), Python (.py), 
    CMakeLists.txt, and CMake (.cmake) files to ensure they contain 
    the required copyright banner from BANNER.txt.
    """

    def __init__(self, banner_file: str = "BANNER.txt"):
        self.banner_file = banner_file
        self.banner_text = self._load_banner()
        self.cpp_extensions = {'.hpp', '.cpp'}
        self.python_extensions = {'.py'}
        self.cmake_files = {'CMakeLists.txt'}
        self.cmake_extensions = {'.cmake'}
        
    def _load_banner(self) -> str:
        """Load and return the banner text from BANNER.txt"""
        try:
            with open(self.banner_file, 'r', encoding='utf-8') as f:
                return f.read().strip()
        except FileNotFoundError:
            print(f"Error: {self.banner_file} not found!")
            sys.exit(1)
        except Exception as e:
            print(f"Error reading {self.banner_file}: {e}")
            sys.exit(1)
    
    def _get_cpp_banner_pattern(self) -> str:
        """Generate the expected C++ banner pattern"""
        escaped_banner = re.escape(self.banner_text)
        # Allow for whitespace variations in the comment block
        return rf'/\*\*\s*{escaped_banner}\s*\*/'
    
    def _get_python_banner_pattern(self) -> str:
        """Generate the expected Python/CMake banner pattern"""
        # Split banner into lines and prefix each with #
        banner_lines = self.banner_text.split('\n')
        commented_lines = [f'# {line}'.rstrip() if line.strip() else '#' for line in banner_lines]
        commented_banner = '\n'.join(commented_lines)
        escaped_banner = re.escape(commented_banner)
        return escaped_banner
    
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
            
            # For Python files, skip shebang line if present
            lines = content.split('\n')
            start_index = 0
            if file_path.suffix == '.py' and lines and lines[0].startswith('#!'):
                start_index = 1
            
            # Check if banner appears at the start (after potential shebang)
            content_to_check = '\n'.join(lines[start_index:])
            pattern = self._get_python_banner_pattern()
            
            return content_to_check.startswith(self.banner_text.replace('\n', '\n# ').replace('\n# \n', '\n#\n'))
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
            if file_path.is_file() and self._should_check_file(file_path):
                if self.check_file(file_path):
                    compliant_files.append(file_path)
                else:
                    non_compliant_files.append(file_path)
        
        return compliant_files, non_compliant_files
    
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
        description="Check copyright banner compliance in source files"
    )
    parser.add_argument(
        "--banner-file", 
        default="BANNER.txt",
        help="Path to banner text file (default: BANNER.txt)"
    )
    parser.add_argument(
        "--directory",
        default=".",
        help="Root directory to scan (default: current directory)"
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
    
    try:
        checker = BannerChecker(args.banner_file)
    except SystemExit:
        return 1
    
    if args.show_examples:
        checker.generate_banner_examples()
        return 0
    
    print(f"Scanning {args.directory} for copyright banner compliance...")
    print(f"Using banner from: {args.banner_file}")
    print()
    
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
        print("Run with --show-examples to see the expected format.")
        return 1
    else:
        print("All files are compliant! ✓")
        return 0

if __name__ == "__main__":
    sys.exit(main())
