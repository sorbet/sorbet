# typed: true
class A
  extend T::Generic
  E = type_member

  # Repeated argument is typed using element type
  sig(xs: E).void
  def foo(*xs)
    # loads of repeated args become Array of element type
    T.reveal_type(xs) # error: Revealed type: `T::Array[A#E]`
  end
end

# Each repeated argument is checked as element type
A[Integer].new.foo(1, 2, '') # error: `String("")` doesn't match `Integer` for argument `xs`
A[Integer].new.foo(1, 2, 3)
