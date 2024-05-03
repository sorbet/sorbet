# typed: true
# compiled: true
# frozen_string_literal: true

class NotArray
  def size
    "hello there"
  end
end

# Verify that breaking will prevent the type assertion on the result of the call
# to `Array#map`
def main1
  xs = [1,2,3]

  ys = xs.map{|a| break NotArray.new}

  puts ys.size
end

# Verify that not breaking will permit the type assertion on the result of the
# call to `Array#map`
def main2
  xs = [1,2,3]

  ys = xs.map{|a| NotArray.new}
  # The call to @llvm.assume will disappear by this point in the lowered output,
  # and that's what will cause the isa_Array call to stick around.

  puts ys.size
end

main1
main2
