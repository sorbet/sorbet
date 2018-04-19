# typed: strict
def foo
  next 5 # error: No `do` block around `next`
end
