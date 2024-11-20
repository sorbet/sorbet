# typed: true

it '' do
  x = nil
  1.times do
    x = 1
    #   ^ error: Changing the type of a variable is not permitted in loops and blocks
  end
end

x = nil
1.times do
  x = 1
  #   ^ error: Changing the type of a variable is not permitted in loops and blocks
end
