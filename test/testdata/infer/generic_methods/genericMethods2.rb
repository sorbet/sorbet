# typed: strict
class Foo
  type_parameters(:A).sig(
      blk: T.proc(arg0: Integer).returns(T.type_parameter(:A)),
  )
  .returns(T::Array[T.type_parameter(:A)])
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
