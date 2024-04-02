# typed: true

class A; end
class Scope
  X = A
# ^ def: X
end

class Scope
  puts(X)
  #    ^ usage: X
end

puts(A)
