# typed: true

it '' do
  x = nil
  1.times do
    x = 1
    #   ^ error: Changing the type of a variable in a loop is not permitted
  end
end

x = nil
1.times do
  x = 1
  #   ^ error: Changing the type of a variable in a loop is not permitted
end
