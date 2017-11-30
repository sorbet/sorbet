# @typed
def foo
  outer do # error: Method outer does not exist on Object
    return
    inner do
      foo
    end
  end
end
