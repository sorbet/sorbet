#!/bin/bash
set -euo pipefail

payload="$1"

# mark all our internal functions as, well, available_externally :-)
sed -i'.bak' 's/define double @sorbet_/define available_externally double @sorbet_/g' "$payload"
sed -i'.bak' 's/define i64 @sorbet_/define available_externally i64 @sorbet_/g' "$payload"
sed -i'.bak' 's/define i32 @sorbet_/define available_externally i32 @sorbet_/g' "$payload"
sed -i'.bak' 's/define nonnull i64\* @sorbet_/define available_externally nonnull i64\* @sorbet_/g' "$payload"

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


# NOTE: sorbet_getConstantEpoch should not be available_externally, as it is added during the lowerings pass much later
# in the pipeline. As a result, marking it availble_externally causes the implementation to disappear, earlier, and
# inline is broken at the point where it's introduced.
sed -i'.bak' 's/define available_externally i64 @sorbet_getConstantEpoch/define internal i64 @sorbet_getConstantEpoch/g' "$payload"
sed -i'.bak' 's/define available_externally i64 @sorbet_rubyTrue/define internal i64 @sorbet_rubyTrue/g' "$payload"
sed -i'.bak' 's/define nonnull i64\*\* @sorbet_get_sp/define internal nonnull i64** @sorbet_get_sp/g' "$payload"
sed -i'.bak' 's/define available_externally nonnull i64\* @sorbet_push/define internal nonnull i64* @sorbet_push/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_Integer/define internal zeroext i1 @sorbet_isa_Integer/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_TrueClass/define internal zeroext i1 @sorbet_isa_TrueClass/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_FalseClass/define internal zeroext i1 @sorbet_isa_FalseClass/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_NilClass/define internal zeroext i1 @sorbet_isa_NilClass/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_Symbol/define internal zeroext i1 @sorbet_isa_Symbol/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_Float/define internal zeroext i1 @sorbet_isa_Float/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_Untyped/define internal zeroext i1 @sorbet_isa_Untyped/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_Hash/define internal zeroext i1 @sorbet_isa_Hash/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_Array/define internal zeroext i1 @sorbet_isa_Array/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_Regexp/define internal zeroext i1 @sorbet_isa_Regexp/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_String/define internal zeroext i1 @sorbet_isa_String/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_Proc/define internal zeroext i1 @sorbet_isa_Proc/g' "$payload"
sed -i'.bak' 's/define available_externally zeroext i1 @sorbet_isa_RootSingleton/define internal zeroext i1 @sorbet_isa_RootSingleton/g' "$payload"
# When given ruby code like the following:
#   if cond
#     ABC.new(1)
#   else
#     DEF.new(1)
#   end
# We emit something like the following:
#   %1 = <cond>
#   br i1 %1, label %BB1, label %BB2
#   BB1:
#   class = sorbet_i_getRubyClass("ABC")
#   sorbet_i_send(class, "new")
#   BB2:
#   class = sorbet_i_getRubyClass("DEF")
#   sorbet_i_send(class, "new")
# LLVM notices that ABC and DEF are string constants of the same length, and replaces this with:
#   class = sorbet_i_getRubyClass(phi "ABC", "DEF")
#   sorbet_i_send(class, "new")
# This causes an issue, because sorbet_i_getRubyClass can only handle constants, and returns undef when
# it encounters the phi node.
# By marking sorbet_i_send as nomerge, we are telling LLVM not to attempt to dedup calls to sorbet_i_send, and emit
# the original LLVM code instead.
sed -i'.bak' 's/declare i64 @sorbet_i_send\(.*\)$/declare i64 @sorbet_i_send\1 nomerge/g' "$payload"
