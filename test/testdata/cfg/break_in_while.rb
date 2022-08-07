# typed: true
def foo
  while true
    1
    break 2
    dead
  # ^^^^ error: This expression appears after an unconditional return
  end
end
puts foo
