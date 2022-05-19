# typed: true

StaticField1 = T.let(0, Integer)
def example1
  x = StaticField1
  1.times do
    x = false
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
    x = false
  end
end
