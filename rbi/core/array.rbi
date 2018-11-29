# typed: true
class Array < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  def self.[](*arg0); end

  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def &(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def *(arg0); end

  sig do
    params(
        arg0: T::Enumerable[Elem],
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def +(arg0); end

  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def -(arg0); end

  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def <<(arg0); end

  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def [](arg0, arg1=T.unsafe(nil)); end

  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
        arg2: Elem,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: T::Range[Integer],
        arg1: Elem,
    )
    .returns(Elem)
  end
  def []=(arg0, arg1, arg2=T.unsafe(nil)); end

  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def assoc(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Elem)
  end
  def at(arg0); end

  sig {returns(T::Array[Elem])}
  def clear(); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(Enumerator[Elem])}
  def collect(&blk); end

  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Elem])
  end
  def combination(arg0, &blk); end

  # This is implemented in C++ to fix the return type
  sig {returns(T::Array[T.untyped])}
  def compact(); end

  sig {returns(T::Array[Elem])}
  def compact!(); end

  sig do
    type_parameters(:T).params(
        arrays: T::Array[T.type_parameter(:T)],
    )
    .returns(T::Array[T.any(Elem, T.type_parameter(:T))])
  end
  def concat(arrays); end

  sig {returns(Integer)}
  sig do
    params(
        arg0: Elem,
    )
    .returns(Integer)
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Integer)
  end
  def count(arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Elem])
  end
  def cycle(arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: Elem,
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Elem,
        blk: T.proc.params().returns(Elem),
    )
    .returns(Elem)
  end
  def delete(arg0, &blk); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.nilable(Elem))
  end
  def delete_at(arg0); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def delete_if(&blk); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def drop(arg0); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def drop_while(&blk); end

  sig {returns(Enumerator[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  def each(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def each_index(&blk); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def empty?(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Integer).returns(Elem),
    )
    .returns(Elem)
  end
  def fetch(arg0, arg1=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Elem,
        arg1: Integer,
        arg2: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Elem,
        arg1: T::Range[Integer],
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(Elem),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
        blk: T.proc.params(arg0: Integer).returns(Elem),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: T::Range[Integer],
        blk: T.proc.params(arg0: Integer).returns(Elem),
    )
    .returns(T::Array[Elem])
  end
  def fill(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil), &blk); end

  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def first(arg0=T.unsafe(nil)); end

  # This is implemented in C++ to fix the return type
  sig {params(depth: Integer).returns(T::Array[T.untyped])}
  def flatten(depth = -1); end

  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def include?(arg0); end

  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(Integer)
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Integer)
  end
  sig {returns(Enumerator[Elem])}
  def index(arg0=T.unsafe(nil), &blk); end

  sig {returns(Object)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Object)
  end
  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .void
  end
  def initialize(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .returns(T::Array[Elem])
  end
  def insert(arg0, *arg1); end

  sig {returns(String)}
  def inspect(); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def join(arg0=T.unsafe(nil)); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  def keep_if(&blk); end

  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def last(arg0=T.unsafe(nil)); end

  sig {returns(Integer)}
  def length(); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(Enumerator[Elem])}
  def map(&blk); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(Enumerator[Elem])}
  def map!(&blk); end

  sig do
    params(
        arg0: Elem,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def member?(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  def permutation(arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig {returns(T.nilable(Elem))}
  def pop(arg0=T.unsafe(nil)); end

  sig do
    type_parameters(:U).params(
        arg0: T::Array[T.type_parameter(:U)],
    )
    .returns(T::Array[T::Array[T.any(Elem, T.type_parameter(:U))]])
  end
  def product(*arg0); end

  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def push(*arg0); end

  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T.nilable(Elem))
  end
  def rassoc(arg0); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def reject(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def reject!(&blk); end

  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Elem])
  end
  def repeated_combination(arg0, &blk); end

  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Elem])
  end
  def repeated_permutation(arg0, &blk); end

  sig {returns(T::Array[Elem])}
  def reverse(); end

  sig {returns(T::Array[Elem])}
  def reverse!(); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def reverse_each(&blk); end

  sig do
    params(
        arg0: Elem,
    )
    .returns(T.nilable(Integer))
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(Integer))
  end
  sig {returns(Enumerator[Elem])}
  def rindex(arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def rotate(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def rotate!(arg0=T.unsafe(nil)); end

  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def sample(arg0=T.unsafe(nil)); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def select(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def select!(&blk); end

  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def shift(arg0=T.unsafe(nil)); end

  sig {returns(T::Array[Elem])}
  def shuffle(); end

  sig {returns(T::Array[Elem])}
  def shuffle!(); end

  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(T.nilable(Elem))
  end
  def slice!(arg0, arg1=T.unsafe(nil)); end

  sig {returns(T::Array[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def sort(&blk); end

  sig {returns(T::Array[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def sort!(&blk); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def sort_by!(&blk); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def take(arg0); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def take_while(&blk); end

  sig {returns(T::Array[Elem])}
  def to_a(); end

  sig {returns(T::Array[Elem])}
  def to_ary(); end

  sig {returns(T::Array[Elem])}
  def transpose(); end

  sig {returns(T::Array[Elem])}
  def uniq(); end

  sig {returns(T::Array[Elem])}
  def uniq!(); end

  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def unshift(*arg0); end

  sig do
    params(
        arg0: T.any(T::Range[Integer], Integer),
    )
    .returns(T::Array[Elem])
  end
  def values_at(*arg0); end

  sig do
    type_parameters(:U).params(
        arg0: T::Array[T.type_parameter(:U)],
    )
    .returns(T::Array[[Elem, T.type_parameter(:U)]])
  end
  def zip(*arg0); end

  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def |(arg0); end

  sig {returns(Integer)}
  def size(); end

  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def slice(arg0, arg1=T.unsafe(nil)); end

  # {#sum} will combine non-{Numeric} {Elem} types using `:+`, the sigs here
  # assume any custom implementation of `:+` is sane and returns the same type
  # as the receiver.
  #
  # @note Since `[].sum` is `0`, {Integer} is a potential return type even if it
  #   is incompatible to sum with the {Elem} type.
  #
  # @example returning {Integer}
  #   T::Array[Float].new.sum #=> 0
  # @example returning {Elem}
  #   [1.0].sum #=> 1.0
  Sorbet.sig {returns(T.any(Elem, Integer))}
  # {#sum} can optionally take a block to perform a function on each element of
  # the receiver before summation. (It will still return `0` for an empty
  # array.)
  #
  # @example returning {Integer}
  #   T::Array[Float].new.sum(&:to_f) #=> 0
  # @example returing generic type
  #   ['a', 'b'].sum{|t| t.ord.to_f} #=> 195.0
  Sorbet.sig do
    type_parameters(:T).params(
      blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:T))
    ).returns(T.any(Integer, T.type_parameter(:T)))
  end
  # The generic is probably overkill here, but `arg0` is returned when the
  # receiver is empty, even if `arg0` and the {Elem} type cannot be combined
  # with `:+`.
  #
  # @example returning {Elem} type
  #   [1.0].sum(1) #=> 2.0
  # @example returning generic type
  #   T::Array[Float].new.sum(1) #=> 1
  Sorbet.sig do
    type_parameters(:T)
      .params(arg0: T.type_parameter(:T))
      .returns(T.any(Elem, T.type_parameter(:T)))
  end
  # In the most general case, {#sum} can take both an initial value and a block
  # to process each element. We require `arg0` and the `blk` return type to
  # match, but ruby does not require this.
  #
  # @example
  #   [1.0].sum(1) {|t| t.to_i}
  # @example this is valid ruby via coercion but does not typecheck
  #   T::Array[Float].new.sum(1) {|t| Complex(t, 1)} #=> 1
  # @example this is valid ruby via coercion but does not typecheck
  #   [Complex(1, 2)].sum(1) {|t| 1.0} #=> 2.0
  Sorbet.sig do
    type_parameters(:U).params(
      arg0: T.type_parameter(:U),
      blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U))
    ).returns(T.type_parameter(:U))
  end
  def sum(arg0=T.unsafe(0), &blk); end

  sig {returns(String)}
  def to_s(); end
end
