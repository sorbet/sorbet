# typed: true

# overriding a method not marked overridable without a sig should be okay
class A1
  def foo; end
end
class B1 < A1
  extend T::Sig
  sig { override.void }
  def foo; end
end

# overriding a method not marked overridable with a sig should be okay
class A2
  extend T::Sig
  sig { void }
  def foo; end
end
class B2 < A2
  extend T::Sig
  sig { override.void }
  def foo; end
end

# overriding an overridable method but not using a sig on the child should be okay
class A3
  extend T::Sig
  sig { overridable.void }
  def foo; end
end
class B3 < A3
  def foo; end
end

# overriding an overridable method with a non-override sig on the child should be an error
class A4
  extend T::Sig
  sig { overridable.void }
  def foo; end
end
class B4 < A4
  extend T::Sig
  sig { void }
  def foo; end # error: Method `B4#foo` overrides an overridable method `A4#foo` but is not declared with `override.`
end

# overriding an overridable method with an override sig on the child should be okay
class A5
  extend T::Sig
  sig { overridable.void }
  def foo; end
end
class B5 < A5
  extend T::Sig
  sig { override.void }
  def foo; end
end

# however, not using override when overriding B5#foo should be an error
class C5 < B5
  extend T::Sig
  sig {void}
  def foo; end
# ^^^^^^^ error: Method `C5#foo` overrides an overridable method
end

class A6
  extend T::Sig
  sig {void}
  def foo; end
end

class B6 < A6
  extend T::Sig
  sig {override.params(x: String).void}
  def foo(x); end
# ^^^^^^^^^^ error: Override of method `A6#foo` requires too many arguments
end

class A7
  extend T::Sig
  sig {void}
  def foo; end
end

class B7 < A7
  extend T::Sig
  sig {override.returns(String)}
  def foo; 'foo' end
end

class Unrelated; end

class A8
  extend T::Sig
  sig {params(x: Integer, y: String).void}
  def foo(x, y); end
end

class B8 < A8
  extend T::Sig
  sig {override.params(x: Integer, y: Integer).void}
#                                  ^ error: Parameter `y` of type `Integer` not compatible with type of overridden method `A8#foo`
  def foo(x, y=0); end
end

class C8 < A8
  extend T::Sig
  sig {override.params(x: String, y: String).void}
#                      ^ error: Parameter `x` of type `String` not compatible with type of overridden method `A8#foo`
  def foo(x="", y); end
end

# should be fine, `z` remains unbound
class D8 < A8
  extend T::Sig
  sig {override.params(x: Integer, y: String, z: Integer).void}
  def foo(x, y="", z=0); end
end

# should be fine, `y` is the tail argument of `A8#foo`
class E8 < A8
  extend T::Sig
  sig {override.params(x: Integer, y: Integer, z: String).void}
  def foo(x, y=0, z); end
end

# should be fine, no arguments added to splat
class F8 < A8
  extend T::Sig
  sig {override.params(x: Integer, y: Unrelated, z: String).void}
  def foo(x, *y, z); end
end

# https://github.com/sorbet/sorbet/issues/8343
class A9
  extend T::Sig

  sig {params(x: Integer).void}
  def foo(x); end
end

class B9 < A9
  sig {override.params(args: String).void}
  #                    ^^^^ error: Parameter `args` of type `String` not compatible with type of overridden method `A9#foo`
  def foo(*args); end
end

class A10
  extend T::Sig
  sig {params(x: Integer, y: String, z: Integer).void}
  def foo(x, y, z); end
end

class B10 < A10
  extend T::Sig
  sig {override.params(x: Integer, y: Integer, z: Integer).void}
  #                                ^ error: Parameter `y` of type `Integer` not compatible with type of overridden method `A10#foo`
  def foo(x, y=0, z); end
end

class C10 < A10
  extend T::Sig
  sig {override.params(x: Integer, y: Integer, z: Integer).void}
  #                                ^ error: Parameter `y` of type `Integer` not compatible with type of overridden method `A10#foo`
  def foo(x, *y, z); end
end

class D10 < A10
  extend T::Sig
  sig {override.params(x: Integer, y: String, z: String).void}
  #                                           ^ error: Parameter `z` of type `String` not compatible with type of overridden method `A10#foo`
  def foo(x, *y, z); end
end

class A11
  extend T::Sig
  sig {params(x: Integer, y: Integer).void}
  def foo(x=0, y); end
end

class B11 < A11
  extend T::Sig
  sig {override.params(x: Integer, y: Integer).void}
  def foo(x, y=0); end
end

class C11 < A11
  extend T::Sig
  # This duplicate error is actually intentional; consider `foo(1)` and `foo(1,1)`.
  sig {override.params(x: String, y: Integer).void}
#                      ^ error: Parameter `x` of type `String` not compatible with type of overridden method `A11#foo`
#                      ^ error: Parameter `x` of type `String` not compatible with type of overridden method `A11#foo`
  def foo(x, y=0); end
end

class D11 < A11
  extend T::Sig
  sig {override.params(x: Integer, y: String).void}
#                                  ^ error: Parameter `y` of type `String` not compatible with type of overridden method `A11#foo`
  def foo(x, *y); end
end
