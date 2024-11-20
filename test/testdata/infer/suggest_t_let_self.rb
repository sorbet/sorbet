# typed: true

x = self

1.times do
  x = 10 # error: Changing the type of a variable is not permitted in loops and blocks
end
