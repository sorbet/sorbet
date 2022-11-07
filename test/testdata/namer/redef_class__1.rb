# typed: true

module A
  X = puts # error: Cannot initialize the class `X` by constant assignment
end
