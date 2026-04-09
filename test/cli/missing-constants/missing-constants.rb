class A < E
  def foo
    B
    Duplicate
  end
  def self.baz
    D
    Duplicate
  end
end

A::C
B

# Qualified constant access where both parts are unresolved
::X::Y.foo
W::Z.bar
