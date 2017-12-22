# @typed
def foo
  [1,2].map do |x|
    3
    break x+1 # error: Method + does not exist on Proc
    4
  end
end
puts foo
