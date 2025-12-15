# typed: true
# compiled: true
# frozen_string_literal: true
# run_filecheck: LOWERED

# NOTE: In LLVM 15, the optimizer is more conservative and doesn't remove
# the @llvm.assume calls or the type tests that feed them.

class NotArray
  def size
    # LOWERED-label: define internal i64 @"func_NotArray#size"
    "hello there"
  end
end

# Verify that breaking will prevent the type assertion on the result of the call
# to `Array#map`
def main1
  # LOWERED-LABEL: define internal i64 @"func_Object#5main1"

  xs = [1,2,3]

  ys = xs.map{|a| break NotArray.new}
  # With break, there's no assume (correct behavior)

  # LOWERED: call i1 @sorbet_i_isa_Array
  puts ys.size
end

# Verify that not breaking will permit the type assertion on the result of the
# call to `Array#map`
def main2
  # LOWERED-LABEL: define internal i64 @"func_Object#5main2"

  xs = [1,2,3]

  ys = xs.map{|a| NotArray.new}
  # In LLVM 15, the @llvm.assume and @sorbet_i_isa_Array calls are preserved.
  # The optimizer is more conservative about removing side-effecting calls.
  # LOWERED: call i1 @sorbet_i_isa_Array
  # LOWERED: @llvm.assume
  puts ys.size
end

main1
main2
