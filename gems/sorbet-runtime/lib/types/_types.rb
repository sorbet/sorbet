# frozen_string_literal: true
# typed: true
# This is where we define the shortcuts, so we can't use them here
# rubocop:disable PrisonGuard/UseOpusTypesShortcut

#  _____
# |_   _|   _ _ __   ___  ___
#   | || | | | '_ \ / _ \/ __|
#   | || |_| | |_) |  __/\__ \
#   |_| \__, | .__/ \___||___/
#       |___/|_|
#
# Docs at https://sorbet.org/docs/sigs
#
# Types that you can pass to `sig`:
#
#  - a Ruby class
#
#  - [<Type>, <Type>, ...] -- to specify a "tuple"; a fixed-size array with known types for each member
#
#  - {key: <Type>, key2: <Type>, ...} -- to speicfy a "shape"; a fixed-size hash
#  with known keys and type values
#
#  - Any of the `T.foo` methods below

module T
  # T.any(<Type>, <Type>, ...) -- matches any of the types listed
  def self.any(type_a, type_b, *types)
    T::Types::Union.new([type_a, type_b] + types)
  end

  # Shorthand for T.any(type, NilClass)
  def self.nilable(type)
    T::Types::Union.new([type, NilClass])
  end

  # Matches any object. In the static checker, T.untyped allows any
  # method calls or operations.
  def self.untyped
    T::Types::Untyped.new
  end

  # Indicates a function never returns (e.g. "Kernel#raise")
  def self.noreturn
    T::Types::NoReturn.new
  end

  # T.all(<Type>, <Type>, ...) -- matches an object that has all of the types listed
  def self.all(type_a, type_b, *types)
    T::Types::Intersection.new([type_a, type_b] + types)
  end

  # Matches any of the listed values
  def self.enum(values)
    T::Types::Enum.new(values)
  end

  # Creates a proc type
  def self.proc
    T::Private::Methods.start_proc
  end

  # Matches `self`:
  def self.self_type
    T::Types::SelfType.new
  end

  # Matches any class that subclasses or includes the provided class
  # or module
  def self.class_of(klass)
    T::Types::ClassOf.new(klass)
  end


  ## END OF THE METHODS TO PASS TO `sig`.


  # Constructs a type alias. Used to create a short name for a larger
  # type. In Ruby this is just equivalent to assignment, but this is
  # needed for support by the static checker. Example usage:
  #
  #  NilableString = T.type_alias(T.nilable(String))
  #
  #  sig {params(arg: NilableString, default: String).returns(String)}
  #  def or_else(arg, default)
  #    arg || default
  #  end
  #
  # The name of the type alias is not preserved; Error messages will
  # be printed with reference to the underlying type.
  def self.type_alias(type)
    T::Utils.coerce(type)
  end

  # References a type paramater which was previously defined with
  # `type_parameters`.
  #
  # This is used for generic methods. Example usage:
  #
  #  sig
  #  .type_parameters(:U)
  #  .params(
  #    blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
  #  )
  #  .returns(T::Array[T.type_parameter(:U)])
  #  def map(&blk); end
  def self.type_parameter(name)
    T::Types::TypeParameter.new(name)
  end

  # Tells the typechecker that `value` is of type `type`. Use this to get additional checking after
  # an expression that the typechecker is unable to analyze. If `checked` is true, raises an
  # exception at runtime if the value doesn't match the type.
  #
  # Compared to `T.let`, `T.cast` is _trusted_ by static system.
  def self.cast(value, type, checked: true)
    return value unless checked

    Private::Casts.cast(value, type, cast_method: "T.cast")
  end

  # Tells the typechecker to declare a variable of type `type`. Use
  # like:
  #
  #  seconds = T.let(0.0, Float)
  #
  # Compared to `T.cast`, `T.let` is _checked_ by static system.
  #
  # If `checked` is true, raises an exception at runtime if the value
  # doesn't match the type.
  def self.let(value, type, checked: true)
    return value unless checked

    Private::Casts.cast(value, type, cast_method: "T.let")
  end

  # Tells the typechecker to ensure that `value` is of type `type` (if not, the typechecker will
  # fail). Use this for debugging typechecking errors, or to ensure that type information is
  # statically known and being checked appropriately. If `checked` is true, raises an exception at
  # runtime if the value doesn't match the type.
  def self.assert_type!(value, type, checked: true)
    return value unless checked

    Private::Casts.cast(value, type, cast_method: "T.assert_type!")
  end

  # For the static type checker, strips all type information from a value
  # and returns the same value, but statically-typed as `T.untyped`.
  # Can be used to tell the static checker to "trust you" by discarding type information
  # you know to be incorrect. Use with care!
  # (This has no effect at runtime.)
  #
  # We can't actually write this sig because we ourselves are inside
  # the `T::` module and doing this would create a bootstrapping
  # cycle. However, we also don't actually need to do so; An untyped
  # identity method works just as well here.
  #
  # sig {params(value: T.untyped).returns(T.untyped)}
  def self.unsafe(value)
    value
  end

  # A convenience method to `raise` when the argument is `nil` and return it
  # otherwise.
  #
  # Intended to be used as:
  #
  #   needs_foo(T.must(maybe_gives_foo))
  #
  # Equivalent to:
  #
  #   foo = maybe_gives_foo
  #   raise "nil" if foo.nil?
  #   needs_foo(foo)
  #
  # Intended to be used to promise sorbet that a given nilable value happens
  # to contain a non-nil value at this point.
  #
  # sig {params(arg: T.nilable(A), msg: T.nilable(String)).returns(A)}
  def self.must(arg, msg=nil)
    begin
      if msg
        if !T.unsafe(msg).is_a?(String)
          raise TypeError.new("T.must expects a string as second argument")
        end
      else
        msg = "Passed `nil` into T.must"
      end
      raise TypeError.new(msg) if arg.nil?
      arg
    rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
      T::Private::ErrorHandler.handle_inline_type_error(e)
      arg
    end
  end

  # A way to ask Sorbet to show what type it thinks an expression has.
  # This can be useful for debugging and checking assumptions.
  # In the runtime, merely returns the value passed in.
  def self.reveal_type(value)
    value
  end

  ### Generic classes ###

  module Array
    def self.[](type)
      T::Types::TypedArray.new(type)
    end
  end

  module Hash
    def self.[](keys, values)
      T::Types::TypedHash.new(keys: keys, values: values)
    end
  end

  module Enumerable
    def self.[](type)
      T::Types::TypedEnumerable.new(type)
    end
  end

  module Enumerator
    def self.[](type)
      T::Types::TypedEnumerator.new(type)
    end
  end

  module Range
    def self.[](type)
      T::Types::TypedRange.new(type)
    end
  end

  module Set
    def self.[](type)
      T::Types::TypedSet.new(type)
    end
  end

  # When mixed into a module, indicates that Sorbet may export the CFG for methods in that module
  module CFGExport; end
end
# rubocop:enable PrisonGuard/UseOpusTypesShortcut
