#!/bin/bash
set -euo pipefail

payload="$1"

# mark all our unternal functions as, well, available_externally :-)
sed -i'.bak' 's/define double @sorbet_/define available_externally double @sorbet_/g' "$payload"
sed -i'.bak' 's/define i64 @sorbet_/define available_externally i64 @sorbet_/g' "$payload"
sed -i'.bak' 's/define i32 @sorbet_/define available_externally i32 @sorbet_/g' "$payload"
sed -i'.bak' 's/define nonnull i64\* @sorbet_/define available_externally nonnull i64\* @sorbet_/g' "$payload"
sed -i'.bak' 's/define %struct\.sorbet_Closure\* @sorbet_/define available_externally %struct\.sorbet_Closure\* @sorbet_/g' "$payload"

sed -i'.bak' 's/define i8\* @sorbet_/define available_externally i8\* @sorbet_/g' "$payload"

sed -i'.bak' 's/define i64\* @sorbet_/define available_externally i64\* @sorbet_/g' "$payload"
sed -i'.bak' 's/define i64\*\* @sorbet_/define available_externally i64\*\* @sorbet_/g' "$payload"
sed -i'.bak' 's/define zeroext i1 @sorbet_/define available_externally zeroext i1 @sorbet_/g' "$payload"
sed -i'.bak' 's/define void @sorbet_/define available_externally void @sorbet_/g' "$payload"

sed -i'.bak' 's/define %struct.rb_control_frame_struct\* @sorbet_/define available_externally %struct.rb_control_frame_struct\* @sorbet_/g' "$payload"

# mark class constants as, well, constant.

# quoting spec
## LLVM explicitly allows declarations of global variables to be marked constant, even if the final definition of the global is not.
## This capability can be used to enable slightly better optimization of the program,
## but requires the language definition to guarantee that optimizations based on the ‘constantness’
## are valid for the translation units that do not include the definition.

sed -i'.bak' 's/@rb_c\(.*\) = external local_unnamed_addr global/@rb_c\1 = external local_unnamed_addr constant/' "$payload"


# remove noise
sed -i'.bak' '/llvm\.module\.flags/d' "$payload"
sed -i'.bak' '/llvm\.ident/d' "$payload"
sed -i'.bak' '/^\^/d' "$payload"

# remove free-form atts
sed -i'.bak' 's/".*"=".*"//g' "$payload"
sed -i'.bak' 's/{  }/{ "addedToSilenceEmptyAttrsError" }/g' "$payload"
