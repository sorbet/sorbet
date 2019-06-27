set -eu
export LC_ALL=C

git ls-files -z '*.out' '*.rb' '*.rbupdate' '*.md' '*.html' '*.rec' |
xargs -0 sed -E -i '' 's/`([^`]+)` does not match `([^`]+)` for argument `([^`]+)`/Expected `\2` but found `\1` for argument `\3`/g'

git ls-files -z '*.out' '*.rb' '*.rbupdate' '*.md' '*.html' '*.rec' |
xargs -0 sed -E -i '' 's/`([^`]+)` does not match `([^`]+)` for block argument/Expected `\2` but found `\1` for block argument/g'
