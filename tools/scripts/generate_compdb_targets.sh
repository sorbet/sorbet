#!/bin/bash
set -e

cd "$(dirname "$0")/../.."

targets="$(bazel query --curses=no --noshow_progress 'kind("cc_(library|binary|test)", //...)' 2>/dev/null)"

buildifier=$(mktemp -t buildifier.XXXXXX)
cleanup() {
    rm -rf "tools/BUILD.tmp"
    rm "$buildifier"
    if [ -e diff.output ]; then
      rm -f diff.output
    fi
}
trap cleanup exit

bazel run \
  --ui_event_filters=-info,-stdout,-stderr \
  --noshow_progress \
  --script_path "$buildifier" \
  @com_github_bazelbuild_buildtools//buildifier &> /dev/null

(
    sed -n '1,/BEGIN compile_commands/p' tools/BUILD
    echo "$targets" | sed -e 's/^/"/' -e 's/$/",/' | grep -v "\\.tar"
    sed -n '/END compile_commands/,$p' tools/BUILD
) | "$buildifier" > tools/BUILD.tmp

if [ "$1" == "-t" ]; then
    if ! diff -u tools/BUILD tools/BUILD.tmp > diff.output; then
        echo "The tools/BUILD file needs to be updated."
        echo "\`\`\`"
        cat diff.output
        echo "\`\`\`"
        echo ""
        echo "Please run \`tools/scripts/generate_compdb_targets.sh\` and commit the result."
        exit 1
    fi
else
    mv -f tools/BUILD.tmp tools/BUILD
fi
