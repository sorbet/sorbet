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
