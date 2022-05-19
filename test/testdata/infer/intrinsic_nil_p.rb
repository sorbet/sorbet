# typed: true
extend T::Sig

sig {params(x: Kernel).void}
def example1(x)
  T.reveal_type(x.nil?) # error: `T::Boolean`
end

sig {params(x: T.nilable(String)).void}
def example2(x)
  T.reveal_type(x.nil?) # error: `T::Boolean`
end

sig {params(x: String).void}
def example3(x)
  T.reveal_type(x.nil?) # error: `FalseClass`

  if x.nil?
    puts 'hello' # error: This code is unreachable
  end
end

sig {params(x: Object).void}
def example4(x)
  T.reveal_type(x.nil?) # error: `T::Boolean`
end

module M
  include Kernel
end
sig {params(x: M).void}
def example5(x)
  T.reveal_type(x.nil?) # error: `T::Boolean`
end

sig {params(x: NilClass).void}
def example6(x)
  T.reveal_type(x.nil?) # error: `TrueClass`
end

sig do
  type_parameters(:U)
    .params(
       x: T.all(Kernel, T.type_parameter(:U)),
       y: T.type_parameter(:U),
    )
    .void
end
def example7(x, y)
  T.reveal_type(x.nil?) # error: `T::Boolean`
  y_nil = y.nil? # error: Call to method `nil?` on unconstrained generic type
  T.reveal_type(y_nil) # error: `T.untyped`
end

class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member
  BoundedElem = type_member {{upper: Kernel}}

  sig {params(x: T.all(Kernel, Elem), y: Elem, z: BoundedElem).void}
  def initialize(x, y, z)
    T.reveal_type(x.nil?) # error: `T::Boolean`
    y_nil = y.nil? # error: Call to method `nil?` on unbounded type member
    T.reveal_type(y_nil) # error: `T.untyped`
    z_nil = z.nil?
    T.reveal_type(z_nil) # error: `T::Boolean`
  end
end
