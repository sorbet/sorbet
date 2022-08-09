# typed: true
def foo
  outer do # error: Method `outer` does not exist on `Object`
    return
    inner do # error: This expression appears after an unconditional return
      foo
    end
  end
end
