#!/bin/bash
set -euo pipefail

payload="$1"

# mark all our unternal functions as, well, internal :-)
sed -i '.bak' 's/define double @sorbet_/define internal double @sorbet_/g' "$payload"
sed -i '.bak' 's/define i64 @sorbet_/define internal i64 @sorbet_/g' "$payload"
sed -i '.bak' 's/define i8\* @sorbet_/define internal i8\* @sorbet_/g' "$payload"
sed -i '.bak' 's/define zeroext i1 @sorbet_/define internal zeroext i1 @sorbet_/g' "$payload"
sed -i '.bak' 's/define void @sorbet_/define internal void @sorbet_/g' "$payload"


# remove noise

sed -i '.bak' '/llvm\.module\.flags/d' "$payload"
sed -i '.bak' '/llvm\.ident/d' "$payload"
sed -i '.bak' '/^\^/d' "$payload"

# remove free-form atts
sed -i '.bak' 's/".*"=".*"//g' "$payload"
sed -i '.bak' 's/{  }/{ "addedToSilenceEmptyAttrsError" }/g' "$payload"
