#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/../.."

if [ "${1:-}" = "" ] || [ "${1:-}" = "-h" ] || [ "${1:-}" = "" ]; then
  >&2 echo "usage: $0 <slug>"
  >&2 echo
  >&2 echo "Initializes a post at website/docs/<slug>.md"
  [ "${1:-}" = "" ]
  exit
fi

slug="$1"
newdoc="website/docs/$slug.md"
sidebars_json="website/sidebars.json"

cat > "$newdoc" <<EOF
---
id: $slug
title: TODO
# sidebar_label: TODO
---

EOF

# shellcheck disable=SC1003
sed -i.bak -e '/Type System/a\'$'\n''      "'"$slug"'",' "$sidebars_json"
rm -f "$sidebars_json.bak"

echo "Done:"
echo "- $newdoc"
echo "- $sidebars_json"

if [ "${EDITOR:-}" != "" ]; then
  editor_command=("$EDITOR")
  if [[ "$EDITOR" =~ "vi" ]]; then
    editor_command+=("-p")
  fi
  exec "${editor_command[@]}" "$newdoc" "$sidebars_json"
fi
