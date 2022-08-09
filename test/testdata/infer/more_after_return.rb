# typed: true

def foo
  return 1
  more_stuff
# ^^^^^^^^^^ error: This expression appears after an unconditional return
end
