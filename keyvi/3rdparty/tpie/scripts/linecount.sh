#!/bin/sh
cd "`dirname "$0"`"/..

# List all files in source control
git ls-tree -r HEAD | cut -f 2 | \

# Remove unused files
grep -v '^tpie/deadcode/\|^apps/' | \

# Run git blame on all files
xargs -n 1 git blame HEAD -- | \

# Remove lines beginning with '/' or '#'
grep '^[^(]*([^)]*) *[^ /#]' | \

# Extract author name from git blame output
sed -rne 's/.*\(([^)]*[^) ]) *....-..-.. ..:..:.. \+.... *[0-9]*\).*$/\1/ p' | \

# Count occurrences of each author name
sort | uniq -c | sort -rn
