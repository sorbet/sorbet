# typed: true

# This test at one point failed because we make use of the Array#[] intrinsic
# in one of the steps to load the types of block arguments

class A
  extend T::Sig
  extend T::Generic
  include Enumerable

  Elem = type_member {{fixed: String}}

  sig do
    override.
    params(
        blk: T.proc.params(arg0: Elem).void,
    )
    .returns(T.untyped)
  end
  def each(&blk) # error: Block parameter `blk` of type `T.proc.params(arg0: A::Elem).void` not compatible with type of abstract method `Enumerable#each`
    yield ''
  end

  sig {params(xs: T::Array[Integer]).void}
  def example(xs)
    xs.each do |x|
      T.reveal_type(x) # error: `Integer`
      x.even?
    end
  end
end

