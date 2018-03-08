# @typed
def foo
  [1].map do |x|
    good # error: Method `good` does not exist on `Object`
    next x
    bad
  end
end
