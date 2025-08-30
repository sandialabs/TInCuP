#!/bin/bash

echo "🔧 Setting up TInCuP git hooks..."

# Pre-commit hook that runs checkin.sh
cat > .git/hooks/pre-commit << 'HOOK'
#!/bin/bash

echo "🔍 Running TInCuP pre-commit checks..."

# Skip checks for WIP commits
if git log -1 --pretty=%B 2>/dev/null | grep -q "^WIP\|^wip\|fixup!\|squash!"; then
    echo "🚧 WIP commit detected, skipping pre-commit checks"
    exit 0
fi

if ! ./scripts/checkin.sh; then
    echo "❌ Pre-commit checks failed. Commit blocked."
    echo "💡 Fix the issues above and try committing again."
    echo "💡 Or use 'git commit --no-verify' to bypass checks."
    echo "💡 Or use 'WIP: message' to skip checks for work-in-progress commits."
    exit 1
fi

echo "✅ Pre-commit checks passed!"
HOOK

# Prepare-commit-msg hook that automatically adds DCO signoff
cat > .git/hooks/prepare-commit-msg << 'HOOK'
#!/bin/bash

# Add DCO signoff if not present
if ! grep -q "Signed-off-by:" "$1"; then
    echo "" >> "$1"
    echo "Signed-off-by: $(git config user.name) <$(git config user.email)>" >> "$1"
fi
HOOK

# Make hooks executable
chmod +x .git/hooks/pre-commit
chmod +x .git/hooks/prepare-commit-msg

echo "✅ Git hooks installed successfully!"
echo ""
echo "📋 What was installed:"
echo "  • pre-commit: Runs ./scripts/checkin.sh before every commit"
echo "  • prepare-commit-msg: Automatically adds DCO signoff to commit messages"
echo ""
echo "🎯 Usage:"
echo "  • Normal commits: git commit -m 'message' (DCO added automatically)"
echo "  • Skip checks: git commit --no-verify (emergency use only)"
echo "  • WIP commits: git commit -m 'WIP: message' (skips checkin.sh but still adds DCO)"
echo ""
echo "💡 All team members should run: ./scripts/setup-git-hooks.sh"
