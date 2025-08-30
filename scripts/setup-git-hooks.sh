#!/bin/bash

echo "Setting up git hooks for TInCuP..."

# Pre-commit hook
cat > .git/hooks/pre-commit << 'HOOK'
#!/bin/bash

echo "🔍 Running TInCuP pre-commit checks..."

if ! ./scripts/checkin.sh; then
    echo "❌ Pre-commit checks failed. Commit blocked."
    echo "💡 Run './scripts/checkin.sh' manually to see details."
    exit 1
fi

echo "✅ All checks passed!"
HOOK

chmod +x .git/hooks/pre-commit

echo "✅ Git hooks installed successfully!"
echo "💡 Use 'git commit --no-verify' to bypass checks if needed."
