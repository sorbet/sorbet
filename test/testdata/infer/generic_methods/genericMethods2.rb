# typed: true
class Foo
  extend T::Generic

  sig do
    type_parameters(:A).params(
        blk: T.proc.params(arg0: Integer).returns(T.type_parameter(:A)),
    )
    .returns(T::Array[T.type_parameter(:A)])
  end
  def map(&blk);
    [blk.call(1)]
  end
end

foo = Foo.new
T.let(foo.map do |n|
    n + 1
  end,
  T::Array[Integer]
)
