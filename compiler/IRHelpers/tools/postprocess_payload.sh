#!/bin/bash
set -euo pipefail

payload="$1"
sed -i '.bak' 's/define double @sorbet_/define internal double @sorbet_/g' "$1"
sed -i '.bak' 's/define i64 @sorbet_/define internal i64 @sorbet_/g' "$1"
sed -i '.bak' 's/define i8\* @sorbet_/define internal i8\* @sorbet_/g' "$1"
sed -i '.bak' 's/define void @sorbet_/define internal void @sorbet_/g' "$1"
sed -i '.bak' 's/define zeroext i1 @sorbet_/define internal zeroext i1 @sorbet_/g' "$1"
