#!/bin/bash
set -x
rec_to_be_updated="$1"
runner="$2"

updated="$("${runner}" "${rec_to_be_updated}" update)"
echo "${updated}" > "${rec_to_be_updated}"
