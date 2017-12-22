# @typed
def foo
  [1].map do |x|
    good # error: Method good does not exist on Object
    next x+1 # error: Method + does not exist on Proc
    bad
  end
end
