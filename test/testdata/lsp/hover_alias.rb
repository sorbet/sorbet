# typed: true

# The documentation for `A` itself
class A; end
class Scope
  # The documentation for this alias
  X = A
end

class Scope
  puts(X)
  #    ^ hover: The documentation for this alias
  #    ^ hover: The documentation for `A` itself
end
