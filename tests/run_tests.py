#!/usr/bin/env python3
# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

"""
Test runner for CPO pattern verification system.

Usage:
    python run_tests.py                    # Run all tests
    python run_tests.py --pattern          # Run only pattern tests  
    python run_tests.py --verification     # Run only verification tests
    python run_tests.py --verbose          # Verbose output
    python run_tests.py --coverage         # Run with coverage (if available)
"""

import sys
import os
import argparse
import unittest

# Add cpo_tools to path for imports  
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../cpo_tools'))

def main():
    parser = argparse.ArgumentParser(description='Run CPO pattern verification tests')
    parser.add_argument('--pattern', action='store_true', 
                       help='Run only pattern generation tests')
    parser.add_argument('--verification', action='store_true',
                       help='Run only verification tests') 
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Verbose test output')
    parser.add_argument('--coverage', action='store_true',
                       help='Run with coverage reporting')
    
    args = parser.parse_args()
    
    # Import test modules
    try:
        from test_cpo_patterns import create_test_suite
        suite = create_test_suite()
    except ImportError as e:
        print(f"Error importing test modules: {e}", file=sys.stderr)
        return 1
    
    # Set up test runner
    verbosity = 2 if args.verbose else 1
    runner = unittest.TextTestRunner(
        verbosity=verbosity,
        buffer=True,  # Capture stdout/stderr during tests
        failfast=False
    )
    
    # Run tests with coverage if requested
    if args.coverage:
        try:
            import coverage
            cov = coverage.Coverage()
            cov.start()
            
            result = runner.run(suite)
            
            cov.stop()
            cov.save()
            
            print("\nCoverage Report:")
            cov.report()
            
        except ImportError:
            print("Coverage module not available. Install with: pip install coverage", 
                  file=sys.stderr)
            return 1
    else:
        result = runner.run(suite)
    
    return 0 if result.wasSuccessful() else 1

if __name__ == '__main__':
    sys.exit(main())