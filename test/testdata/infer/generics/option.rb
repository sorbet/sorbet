# typed: strong

class Module
  include T::Sig
end

module Option
  extend T::Generic
  sealed!
  abstract!

  Elem = type_member(:out)

  sig do
    abstract
      .type_parameters(:U)
      .params(blk: T.proc.params(x: Elem).returns(T.type_parameter(:U)))
      .returns(Option[T.type_parameter(:U)])
  end
  def map(&blk); end

  sig do
    abstract
      .type_parameters(:U)
      .params(blk: T.proc.params(x: Elem).returns(Option[T.type_parameter(:U)]))
      .returns(Option[T.type_parameter(:U)])
  end
  def and_then(&blk); end

  sig {abstract.returns(Elem)}
  def unwrap!(&blk); end

  sig {abstract.params(msg: String).returns(Elem)}
  def expect!(msg, &blk); end

  class Some < T::Struct
    extend T::Generic
    include Option
    Elem = type_member

    prop :val, Elem

    sig do
      override
        .type_parameters(:U)
        .params(blk: T.proc.params(x: Elem).returns(T.type_parameter(:U)))
        .returns(Some[T.type_parameter(:U)])
    end
    def map(&blk)
      new_val = yield self.val
      T.reveal_type(new_val) # error: `T.type_parameter(:U) (of Option::Some#map)`
      res = Some[T.type_parameter(:U)].new(val: new_val)
      T.reveal_type(res) # error: `Option::Some[T.type_parameter(:U) (of Option::Some#map)]`
      res
    end

    sig do
      override
        .type_parameters(:U)
        .params(blk: T.proc.params(x: Elem).returns(Option[T.type_parameter(:U)]))
        .returns(Option[T.type_parameter(:U)])
    end
    def and_then(&blk)
      new_opt = yield self.val
      T.reveal_type(new_opt) # error: `Option[T.type_parameter(:U) (of Option::Some#and_then)]`
      new_opt
    end

    sig {override.returns(Elem)}
    def unwrap!(&blk)
      T.reveal_type(self.val) # error: `Option::Some::Elem`
    end

    sig {override.params(msg: String).returns(Elem)}
    def expect!(msg, &blk)
      T.reveal_type(self.val) # error: `Option::Some::Elem`
    end
  end

  class None < T::Struct
    extend T::Generic
    include Option
    Elem = type_member {{fixed: T.noreturn}}

    sig do
      override
        .type_parameters(:U)
        .params(blk: T.proc.params(x: Elem).returns(T.type_parameter(:U)))
        .returns(None)
    end
    def map(&blk)
      T.reveal_type(self) # error: `Option::None`
    end

    sig do
      override
        .type_parameters(:U)
        .params(blk: T.proc.params(x: Elem).returns(Option[T.type_parameter(:U)]))
        .returns(Option[T.type_parameter(:U)])
    end
    def and_then(&blk)
      T.reveal_type(self) # error: `Option::None`
    end

    sig {override.returns(T.noreturn)}
    def unwrap!(&blk)
      raise ArgumentError.new("Called Option#unwrap! on a None value")
    end

    sig {override.params(msg: String).returns(Elem)}
    def expect!(msg, &blk)
      raise ArgumentError.new(msg)
    end
  end
end

sig {params(maybe_int: Option[Integer]).void}
def example(maybe_int)
  maybe_str = maybe_int.map do |int|
    T.reveal_type(int) # error: `Integer`
    T.reveal_type(int.to_s) # error: `String`
  end
  T.reveal_type(maybe_str) # error: `Option[String]`

  res = maybe_str.and_then do |str|
    T.reveal_type(str) # error: `String`
    if str.empty?
      Option::None.new
    else
      Option::Some[Integer].new(val: str.length.even?)
      #                              ^^^^^^^^^^^^^^^^ error: Expected `Integer` but found `T::Boolean` for argument `val`
    end
  end

  T.reveal_type(res) # error: `Option[Integer]`

  maybe_str.and_then {|str| Option::None} # error: Expected `Option[T.type_parameter(:U)]` but found `T.class_of(Option::None)` for block result type

  1.times do |x|
    Option::None.new.unwrap!
    puts x # error: This code is unreachable
  end

  case maybe_int
  when Option::Some
    T.reveal_type(maybe_int.val) # error: `Integer`
  when Option::None
    T.reveal_type(maybe_int) # error: `Option::None`
  else
    T.absurd(maybe_int)
  end

  case maybe_int
  when Option::None
    T.reveal_type(maybe_int) # error: `Option::None`
  when Option::Some
    T.reveal_type(maybe_int.val) # error: `Integer`
  else
    T.absurd(maybe_int)
  end

  case maybe_int
  when Option::Some
  else
    T.absurd(maybe_int) # error: the type `Option::None` wasn't handled
  end

  case maybe_int
  when Option::None
  else
    T.absurd(maybe_int) # error: the type `Option::Some[Integer]` wasn't handled
  end
end

sig do
  type_parameters(:U, :V)
    .params(
      x: Option[T.type_parameter(:U)],
      blk: T.proc.params(y: T.type_parameter(:U)).returns(T.type_parameter(:V))
    )
    .returns(Option[T.type_parameter(:V)])
end
def map_over_option(x, &blk)
  x.map(&blk)
end

maybe_is_even = map_over_option(Option::Some[Integer].new(val: 0)) do |x|
  T.reveal_type(x.even?) # error: `T::Boolean`
end
T.reveal_type(maybe_is_even) # error: `Option[T::Boolean]`
