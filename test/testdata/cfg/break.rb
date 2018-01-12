# @typed
def foo
  [1,2].map do |x|
    3
    break x
    4
  end
end
puts foo
