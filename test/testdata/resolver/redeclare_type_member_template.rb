# typed: true
# disable-fast-path: true
extend T::Sig

class Parent
  extend T::Generic
  extend T::Sig

  X = type_member
  sig {returns(X)}
  def foo; raise "unimplemented"; end
end

class Child1 < Parent
  extend T::Generic

  X = type_template { {fixed: Integer} } # error: `X` must be declared as a type_member (not a type_template) to match the parent

  def main
    x = foo
  end
end

class Child2 < Parent # error: Type `X` declared by parent `Parent` must be re-declared in `Child2`
  extend T::Generic

  def main
    x = foo
  end
end
