# typed: true
def foo
  while true
    good # error: Method `good` does not exist on `Object`
    next value # error: Method `value` does not exist on `Object`
    bad
  # ^^^ error: This expression appears after an unconditional return
  end
end
