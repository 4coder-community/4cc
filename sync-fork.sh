#!/bin/sh

set -e

cd "$(realpath "$(dirname "$0")")"

BRANCH="${1:-develop}"

echo "Syncing $BRANCH from upstream..."
gh repo sync --branch "$BRANCH"

echo "Updating local $BRANCH..."
git fetch origin
git merge --ff-only "origin/$BRANCH"
