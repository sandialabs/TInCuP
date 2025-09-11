# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)


import subprocess
import pytest
from pathlib import Path

# Discover the project root by looking for the .git directory
PROJECT_ROOT = Path(__file__).parent
while not (PROJECT_ROOT / ".git").exists():
    PROJECT_ROOT = PROJECT_ROOT.parent
    if PROJECT_ROOT == PROJECT_ROOT.parent:
        raise FileNotFoundError("Could not find project root directory")

# Directory containing the diagnostic test cases
DIAGNOSTICS_DIR = PROJECT_ROOT / "tests" / "diagnostic_messages"

# Find all C++ files in the diagnostics directory
test_files = list(DIAGNOSTICS_DIR.glob("*.cpp"))

@pytest.mark.parametrize("cpp_file", test_files, ids=[f.name for f in test_files])
def test_diagnostic(cpp_file):
    """
    Compiles a C++ file that is expected to fail and checks for a specific diagnostic message.
    """
    # The expected error is embedded in the C++ source file in a comment block.
    # Example: /* expected_error: some error text */
    expected_error = ""
    with open(cpp_file, "r") as f:
        content = f.read()
        marker = "/* expected_error:";
        if marker in content:
            start = content.find(marker) + len(marker)
            end = content.find("*/", start)
            expected_error = content[start:end].strip()

    assert expected_error, f"No expected_error comment found in {cpp_file}"

    print(f"\n  Testing: {cpp_file.name}\n  Expecting error: \"{expected_error}\"")

    # Compile the C++ file and capture the output
    # We pass an absolute path to the include directory to ensure it's found
    # regardless of where pytest is run.
    # -fsyntax-only is used to check for errors without generating a full build, which is faster.
    command = [
        "g++",
        "-std=c++20",
        f"-I{PROJECT_ROOT}",
        "-fsyntax-only",
        str(cpp_file),
    ]
    
    result = subprocess.run(command, capture_output=True, text=True)

    # Check that compilation failed
    assert result.returncode != 0, f"Compilation of {cpp_file.name} succeeded but was expected to fail."

    # Check that the specific diagnostic message is present in stderr
    stderr_output = result.stderr
    assert expected_error in stderr_output, \
        f"Expected error message not found in compiler output for {cpp_file.name}.\n" \
        f"Expected: '{expected_error}'\n" \
        f"Got: '{stderr_output}'"
