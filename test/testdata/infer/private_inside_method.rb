# typed: true

class A
  def self.foo; end
  def self.bar; private :foo; end
end

# Sorbet needs a static view of the world. We assume that once a method is
# defined, that's its visibility always.

# no runtime error
A.foo

A.bar

# runtime error (private call)
A.foo
