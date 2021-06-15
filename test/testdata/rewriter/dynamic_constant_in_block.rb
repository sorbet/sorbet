# typed: false

class A
  [].each do
    Y = 30
#   ^ error: Assigning constant `Y` inside a block
  end

  foo { ZZZ = 40 }
#       ^^^ error: Assigning constant `ZZZ` inside a block
end