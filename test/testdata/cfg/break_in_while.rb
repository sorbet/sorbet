# @typed
def foo
  while true # error: Changing type of pinned argument, `TrueClass(true)` is not a subtype of `NilClass`
    1
    break 2
    dead
  end
end
puts foo
