# typed: true

StaticField1 = T.let(0, Integer)
def example1
  x = StaticField1
  1.times do
    x = false # error: Changing the type of a variable in a loop is not permitted
  end
end

StaticField2 = T.let(0, Integer)
def example2
  if T.unsafe(nil)
    x = StaticField2
  else
    x = 0
  end
  1.times do
    x = false # error: Changing the type of a variable in a loop is not permitted
  end
end

# (this is just here to force there to be something in the autocorrect output)
x = false
1.times do
  x = true # error: Changing the type of a variable in a loop is not permitted
end
