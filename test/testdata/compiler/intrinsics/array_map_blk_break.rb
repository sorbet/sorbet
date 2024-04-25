# typed: true
# compiled: true
# frozen_string_literal: true
# run_filecheck: LOWERED

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
  # LOWERED-NOT: @llvm.assume

  # LOWERED: call i1 @sorbet_i_isa_Array
  puts ys.size
end

# Verify that not breaking will permit the type assertion on the result of the
# call to `Array#map`
def main2
  # LOWERED-LABEL: define internal i64 @"func_Object#5main2"

  xs = [1,2,3]

  ys = xs.map{|a| NotArray.new}
  # The call to @llvm.assume will disappear by this point in the lowered output,
  # and that's what will cause the isa_Array call to stick around.

  # LOWERED-NOT: call i1 @sorbet_i_isa_Array
  puts ys.size
end

main1
main2
