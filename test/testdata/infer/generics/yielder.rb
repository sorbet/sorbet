# typed: strict
require_relative './gems/sorbet-runtime/lib/sorbet-runtime'

module IBox
  extend T::Sig
  extend T::Generic
  include Enumerable
  abstract!

  Elem = type_member(:out)

  sig {abstract.returns(Elem)}
  def val; end

  sig do
    override
      .params(
        blk: T.nilable(T.proc.params(arg0: Elem).returns(BasicObject))
      )
      # better sig here would be (if sorbet allowed overloading) to have
      # Box[Elem] in the one case, and T::Enumerator[Elem] in the other
      .returns(T::Enumerable[Elem])
  end
  def each(&blk)
    if Kernel.block_given?
      yield self.val
      self
    else
      T::Enumerator[Elem].new {|yielder| yielder << self.val}
    end
  end
end

class Box < T::Struct
  extend T::Generic
  include IBox
  Elem = type_member
  const :val, Elem
end

T.let(nil, T.nilable(Box))

int_box = Box[Integer].new(val: 0)

res = T.reveal_type(int_box.each) # error: `T::Enumerable[Integer]`
p res
# (arrays can be more specific, because of overloading)
res = T.reveal_type([0].each) # error: `T::Enumerator[Integer]`
p res

res = T.reveal_type(int_box.each.map {|x| x.to_s}) # error: `T::Array[String]`
p res
res = T.reveal_type([0].each.map {|x| x.to_s}) # error: `T::Array[String]`
p res

res = int_box.each do |val|
  T.reveal_type(val) # error: `Integer`
end
T.reveal_type(res) # error: `T::Enumerable[Integer]`
p res

res = [0].each do |val|
  T.reveal_type(val) # error: `Integer`
end
T.reveal_type(res) # error: `T::Array[Integer]`
p res
