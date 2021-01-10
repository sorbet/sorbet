# typed: true

class Parent
  extend T::Sig
  sig {params(x: Integer).void}
  def self.takes_integer_static(x); end

  sig {params(x: Integer).void}
  def takes_integer_instance(x); end
end

class Child < Parent
  takes_integer_static def self.outer_static # error: Expected `Integer` but found `Symbol(:outer_static)` for argument `x`
    takes_integer_static def self.inner_static; end # error: Expected `Integer` but found `Symbol(:inner_static)` for argument `x`
  end

  takes_integer_static def outer_instance # error: Expected `Integer` but found `Symbol(:outer_instance)` for argument `x`
    takes_integer_instance def inner_instance; end # error: Expected `Integer` but found `Symbol(:inner_instance)` for argument `x`
  end
end
