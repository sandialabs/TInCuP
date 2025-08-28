# PowerShell JSON Handling in CI

This document explains the proper way to handle JSON parameters in PowerShell for GitHub Actions CI, specifically for the MSVC CI job.

## The Problem

When passing JSON strings to command-line tools in PowerShell within GitHub Actions, improper escaping can cause JSON parsing errors.

### ❌ Problematic Approach
```powershell
# This fails because the JSON escaping gets mangled
cpo-generator '{\"cpo_name\": \"test\", \"args\": [\"$T&: arg\"]}' --doxygen
```

**Error Result:**
```
Error: Invalid JSON input provided.
'{\"cpo_name\": \"test\", \"args\": [\"$T&: arg\"]}'
```

The escaping becomes corrupted and the JSON parser receives malformed input.

## ✅ Correct Solution

Use PowerShell variables to store the JSON string properly:

```powershell
# Store JSON in a PowerShell variable with proper quoting
$jsonInput = '{"cpo_name": "test", "args": ["$T&: arg"]}'
cpo-generator $jsonInput --doxygen
```

**Why This Works:**
1. Single quotes in PowerShell preserve the string literally
2. No additional escaping is needed for JSON quotes
3. The variable expansion passes the JSON correctly to the command

## Implementation in MSVC CI

### Before (Broken)
```yaml
- name: Test CPO generation
  shell: pwsh
  run: |
    cpo-generator '{\"cpo_name\": \"test_msvc\", \"args\": [\"$T&: arg\"]}' --doxygen
```

### After (Fixed)
```yaml
- name: Test CPO generation
  shell: pwsh
  run: |
    $jsonInput = '{"cpo_name": "test_msvc", "args": ["$T&: arg"]}'
    cpo-generator $jsonInput --doxygen
```

## Best Practices for PowerShell + JSON in CI

### 1. Use PowerShell Variables
Always store JSON in variables instead of inline strings:
```powershell
$json = '{"key": "value"}'
command $json
```

### 2. Single Quotes for JSON Literals
Use single quotes to prevent PowerShell from interpreting special characters:
```powershell
# Good - literal string
$json = '{"name": "test", "args": ["$T&: value"]}'

# Bad - PowerShell interprets $T as variable
$json = "{'name': 'test', 'args': ['$T&: value']}"
```

### 3. Test JSON Validity
Add validation when developing:
```powershell
$jsonInput = '{"cpo_name": "test", "args": ["$T&: arg"]}'
# Test parsing (optional, for debugging)
$parsed = ConvertFrom-Json $jsonInput
Write-Host "JSON is valid: $($parsed.cpo_name)"
```

### 4. Escape Special Characters Only When Needed
In PowerShell single-quoted strings, only the single quote needs escaping:
```powershell
# If JSON contains single quotes, escape them
$json = '{"message": "Don''t break the JSON"}'
```

## Testing PowerShell JSON Syntax

Use the provided test script to verify syntax:
```powershell
# Run the PowerShell test
pwsh tests/test_powershell_json.ps1
```

This script validates:
- JSON syntax is correct
- PowerShell parsing works
- Command construction is proper

## GitHub Actions Context

In GitHub Actions with `shell: pwsh`, the PowerShell environment:
- Uses PowerShell Core (7.x) on all platforms
- Supports cross-platform JSON handling
- Requires proper variable quoting as shown above

## Common Pitfalls

### 1. Double Escaping
```powershell
# Wrong - double escaping
$json = '{\\\"key\\\": \\\"value\\\"}'

# Right - single quotes, no escaping
$json = '{"key": "value"}'
```

### 2. Variable Expansion in JSON
```powershell
# Wrong - $T gets expanded by PowerShell
$json = "{'args': ['$T&: arg']}"

# Right - literal string with single quotes
$json = '{"args": ["$T&: arg"]}'
```

### 3. Here-Strings for Complex JSON
For complex JSON, use here-strings:
```powershell
$json = @'
{
  "cpo_name": "complex_example",
  "args": [
    "$T&: target",
    "const U&: source"
  ]
}
'@
```

## Validation

The CI includes automatic validation:
- `tests/test_msvc_ci.sh` checks for proper PowerShell JSON syntax
- Prevents regression to problematic escaping patterns
- Ensures CI reliability across platforms

This approach ensures reliable JSON handling in PowerShell CI environments.