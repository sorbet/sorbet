# typed: true
class Range < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  sig do
    type_parameters(:U).params(
      from: T.type_parameter(:U),
      to: T.type_parameter(:U),
      exclude_end: T::Boolean
    ).returns(T::Range[T.type_parameter(:U)])
  end
  def self.new(from, to, exclude_end=false); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(obj); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(obj); end

  sig {returns(Elem)}
  def begin(); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T::Boolean),
    )
    .returns(T.nilable(T.type_parameter(:U)))
  end
  def bsearch(&blk); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def cover?(obj); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(Enumerator[Elem])}
  def each(&blk); end

  sig {returns(Elem)}
  def end(); end

  sig {returns(T::Boolean)}
  def exclude_end?(); end

  sig {returns(Elem)}
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  def first(n=T.unsafe(nil)); end

  sig {returns(Integer)}
  def hash(); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def include?(obj); end

  sig do
    params(
        _begin: Elem,
        _end: Elem,
        exclude_end: T::Boolean,
    )
    .void
  end
  def initialize(_begin, _end, exclude_end=T.unsafe(nil)); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(Elem)}
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  def last(n=T.unsafe(nil)); end

  sig {returns(Elem)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(Elem)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def max(n=T.unsafe(nil), &blk); end

  sig {returns(Elem)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(Elem)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def min(n=T.unsafe(nil), &blk); end

  sig {returns(T.nilable(Integer))}
  def size(); end

  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(Enumerator[Elem])
  end
  def step(n=T.unsafe(nil), &blk); end

  sig {returns(String)}
  def to_s(); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(obj); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def member?(obj); end
end
