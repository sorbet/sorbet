# frozen_string_literal: true
# typed: true
# This is where we define the shortcuts, so we can't use them here

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
  # Cache for `T.nilable(<Module>)`, so that inline call sites (e.g.
  # `T.let(x, T.nilable(Integer))`) don't re-derive the union on every call.
  # Keys and values are both weakly referenced: anonymous modules don't leak,
  # and a collected value is simply re-derived. Racy concurrent writes are
  # benign double-initializations, as in T::Types::Simple::Private::Pool.
  @nilable_cache = ObjectSpace::WeakMap.new

  # Two-level cache for `T.any(<Module>, <Module>)` (e.g. inline
  # `T.cast(x, T.any(Integer, Float))`), keyed on the two module literals.
  # Same weak-key/weak-value lifecycle as @nilable_cache; a miss computes
  # through the byte-for-byte existing derivation (including the
  # DuplicateType collapse inside union_of_types, so `T.any(X, X)` still
  # yields Simple(X)) and caches whatever it produces.
  @any_pair_cache = ObjectSpace::WeakMap.new

  # T.any(<Type>, <Type>, ...) -- matches any of the types listed
  def self.any(type_a, type_b, *types)
    if types.empty? && ::Module === type_a && ::Module === type_b
      inner = @any_pair_cache[type_a]
      cached = inner && inner[type_b]
      return cached if cached

      derived = T::Types::Union::Private::Pool.union_of_types(T::Utils.coerce(type_a), T::Utils.coerce(type_b))
      inner ||= (@any_pair_cache[type_a] = ObjectSpace::WeakMap.new)
      inner[type_b] = derived
      derived
    else
      type_a = T::Utils.coerce(type_a)
      type_b = T::Utils.coerce(type_b)
      types = types.map { |t| T::Utils.coerce(t) } if !types.empty?
      T::Types::Union::Private::Pool.union_of_types(type_a, type_b, types)
    end
  end

  # Shorthand for T.any(type, NilClass)
  def self.nilable(type)
    if ::Module === type
      cached = @nilable_cache[type]
      return cached if cached

      # Miss path: compute through the full existing derivation — including
      # the DuplicateType rescue inside union_of_types, so that e.g.
      # `T.nilable(NilClass)` still collapses to `Simple(NilClass)` — and
      # cache whatever it produces.
      @nilable_cache[type] = T::Types::Union::Private::Pool.union_of_types(T::Utils.coerce(type), T::Utils::Nilable::NIL_TYPE)
    else
      # Non-Module inputs (T::Types::Base instances, arrays, hashes, etc.) are
      # typically fresh objects per call, so caching them would only bloat the
      # map without ever hitting.
      T::Types::Union::Private::Pool.union_of_types(T::Utils.coerce(type), T::Utils::Nilable::NIL_TYPE)
    end
  end

  # Matches any object. In the static checker, T.untyped allows any
  # method calls or operations.
  def self.untyped
    T::Types::Untyped::Private::INSTANCE
  end

  # Indicates a function never returns (e.g. "Kernel#raise")
  def self.noreturn
    T::Types::NoReturn::Private::INSTANCE
  end

  def self.anything
    T::Types::Anything::Private::INSTANCE
  end

  # T.all(<Type>, <Type>, ...) -- matches an object that has all of the types listed
  def self.all(type_a, type_b, *types)
    T::Types::Intersection.new([type_a, type_b] + types)
  end

  # Matches any of the listed values
  # @deprecated Use T::Enum instead.
  def self.deprecated_enum(values)
    T::Types::Enum.new(values)
  end

  # Creates a proc type
  def self.proc
    T::Private::Methods.start_proc
  end

  # Matches `self`:
  def self.self_type
    T::Types::SelfType::Private::INSTANCE
  end

  # Matches the instance type in a singleton-class context
  def self.attached_class
    T::Types::AttachedClassType::Private::INSTANCE
  end

  # Cache for `T.class_of(<Module>)`, so that inline call sites don't
  # allocate a fresh ClassOf on every call. Weak keys and values, as above.
  @class_of_cache = ObjectSpace::WeakMap.new

  # Matches any class that subclasses or includes the provided class
  # or module
  def self.class_of(klass)
    if ::Module === klass
      cached = @class_of_cache[klass]
      return cached if cached

      @class_of_cache[klass] = T::Types::ClassOf.new(klass)
    else
      # Non-Module inputs are invalid, but preserve the lazy failure mode:
      # construct the type here and let its use sites raise.
      T::Types::ClassOf.new(klass)
    end
  end

  ## END OF THE METHODS TO PASS TO `sig`.

  # Constructs a type alias. Used to create a short name for a larger type. In Ruby this returns a
  # wrapper that contains a proc that is evaluated to get the underlying type. This syntax however
  # is needed for support by the static checker.
  #
  # @example
  #  NilableString = T.type_alias {T.nilable(String)}
  #
  #  sig {params(arg: NilableString, default: String).returns(String)}
  #  def or_else(arg, default)
  #    arg || default
  #  end
  #
  # The name of the type alias is not preserved; Error messages will
  # be printed with reference to the underlying type.
  #
  # TODO Remove `type` parameter. This was left in to make life easier while migrating.
  def self.type_alias(type=nil, &blk)
    if blk
      T::Private::Types::TypeAlias.new(blk)
    else
      T::Utils.coerce(type)
    end
  end

  # References a type parameter which was previously defined with
  # `type_parameters`.
  #
  # This is used for generic methods.
  #
  # @example
  #  sig
  #  .type_parameters(:U)
  #  .params(
  #    blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
  #  )
  #  .returns(T::Array[T.type_parameter(:U)])
  #  def map(&blk); end
  def self.type_parameter(name)
    T::Types::TypeParameter.make(name)
  end

  # Tells the typechecker that `value` is of type `type`. Use this to get additional checking after
  # an expression that the typechecker is unable to analyze. If `checked` is true, raises an
  # exception at runtime if the value doesn't match the type.
  #
  # Compared to `T.let`, `T.cast` is _trusted_ by static system.
  def self.cast(value, type, checked: true)
    return value unless checked
    # Inlined happy path for Module type literals (e.g. `T.cast(x, Foo)`),
    # hoisted out of Private::Casts.cast to skip its call frame. Failures
    # and non-Module type shapes take the full path below.
    return value if ::Module === type && value.is_a?(type)

    Private::Casts.cast(value, type, "T.cast")
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
    # Inlined happy path for Module type literals; see T.cast.
    return value if ::Module === type && value.is_a?(type)

    Private::Casts.cast(value, type, "T.let")
  end

  # Tells the type checker to treat `self` in the current block as `type`.
  # Useful for blocks that are captured and executed later with instance_exec.
  # Use like:
  #
  #  seconds = lambda do
  #    T.bind(self, NewBinding)
  #    ...
  #  end
  #
  # `T.bind` behaves like `T.cast` in that it is assumed to be true statically.
  #
  # If `checked` is true, raises an exception at runtime if the value
  # doesn't match the type (this is the default).
  def self.bind(value, type, checked: true)
    return value unless checked
    # Inlined happy path for Module type literals; see T.cast.
    return value if ::Module === type && value.is_a?(type)

    Private::Casts.cast(value, type, "T.bind")
  end

  # Tells the typechecker to ensure that `value` is of type `type` (if not, the typechecker will
  # fail). Use this for debugging typechecking errors, or to ensure that type information is
  # statically known and being checked appropriately. If `checked` is true, raises an exception at
  # runtime if the value doesn't match the type.
  def self.assert_type!(value, type, checked: true)
    return value unless checked
    # Inlined happy path for Module type literals; see T.cast.
    return value if ::Module === type && value.is_a?(type)

    Private::Casts.cast(value, type, "T.assert_type!")
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
  # `sig {params(value: T.untyped).returns(T.untyped)}`
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
  # `sig {params(arg: T.nilable(A)).returns(A)}`
  def self.must(arg)
    return arg if arg
    return arg if arg == false

    begin
      raise TypeError.new("Passed `nil` into T.must")
    rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
      T::Configuration.inline_type_error_handler(e, {kind: 'T.must', value: arg, type: nil})
    end
  end

  # A convenience method to `raise` with a provided error reason when the argument
  # is `nil` and return it otherwise.
  #
  # Intended to be used as:
  #
  #   needs_foo(T.must_because(maybe_gives_foo) {"reason_foo_should_not_be_nil"})
  #
  # Equivalent to:
  #
  #   foo = maybe_gives_foo
  #   raise "reason_foo_should_not_be_nil" if foo.nil?
  #   needs_foo(foo)
  #
  # Intended to be used to promise sorbet that a given nilable value happens
  # to contain a non-nil value at this point.
  #
  # `sig {params(arg: T.nilable(A), reason_blk: T.proc.returns(String)).returns(A)}`
  def self.must_because(arg)
    return arg if arg
    return arg if arg == false

    begin
      raise TypeError.new("Unexpected `nil` because #{yield}")
    rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
      T::Configuration.inline_type_error_handler(e, {kind: 'T.must_because', value: arg, type: nil})
    end
  end

  # A way to ask Sorbet to show what type it thinks an expression has.
  # This can be useful for debugging and checking assumptions.
  # In the runtime, merely returns the value passed in.
  def self.reveal_type(value)
    value
  end

  # A way to ask Sorbet to prove that a certain branch of control flow never
  # happens. Commonly used to assert that a case or if statement exhausts all
  # possible cases.
  def self.absurd(value)
    msg = "Control flow reached T.absurd."

    case value
    when Kernel
      msg += " Got value: #{value}"
    end

    begin
      raise TypeError.new(msg)
    rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
      T::Configuration.inline_type_error_handler(e, {kind: 'T.absurd', value: value, type: nil})
    end
  end

  ### Generic classes ###

  module Array
    def self.[](type)
      if type.is_a?(T::Types::Untyped)
        T::Types::TypedArray::Untyped::Private::INSTANCE
      else
        T::Types::TypedArray::Private::Pool.type_for_module(type)
      end
    end
  end

  module Hash
    # Lazily-created shared instance for `T::Hash[T.untyped, T.untyped]`.
    # (Unlike TypedArray's, this can't be a load-time frozen constant:
    # typed_hash.rb is required before T.untyped and T::Utils exist.)
    # A racy double-init stores one of two equivalent frozen instances.
    @untyped_instance = nil

    def self.[](keys, values)
      if keys.is_a?(T::Types::Untyped) && values.is_a?(T::Types::Untyped)
        @untyped_instance ||= T::Types::TypedHash::Untyped.new.freeze
      elsif ::Module === keys && ::Module === values
        T::Types::TypedHash::Private::Pool.type_for_modules(keys, values)
      else
        T::Types::TypedHash.new(keys: keys, values: values)
      end
    end
  end

  module Enumerable
    def self.[](type)
      if type.is_a?(T::Types::Untyped)
        T::Types::TypedEnumerable::Untyped.new
      else
        T::Types::TypedEnumerable.new(type)
      end
    end
  end

  module Enumerator
    def self.[](type)
      if type.is_a?(T::Types::Untyped)
        T::Types::TypedEnumerator::Untyped.new
      else
        T::Types::TypedEnumerator.new(type)
      end
    end

    module Lazy
      def self.[](type)
        if type.is_a?(T::Types::Untyped)
          T::Types::TypedEnumeratorLazy::Untyped.new
        else
          T::Types::TypedEnumeratorLazy.new(type)
        end
      end
    end

    module Chain
      def self.[](type)
        if type.is_a?(T::Types::Untyped)
          T::Types::TypedEnumeratorChain::Untyped.new
        else
          T::Types::TypedEnumeratorChain.new(type)
        end
      end
    end
  end

  module Range
    def self.[](type)
      if type.is_a?(T::Types::Untyped)
        T::Types::TypedRange::Untyped.new
      else
        T::Types::TypedRange.new(type)
      end
    end
  end

  module Set
    def self.[](type)
      if type.is_a?(T::Types::Untyped)
        T::Types::TypedSet::Untyped.new
      else
        T::Types::TypedSet.new(type)
      end
    end
  end

  module Class
    def self.[](type)
      if type.is_a?(T::Types::Untyped)
        T::Types::TypedClass::Untyped::Private::INSTANCE
      elsif type.is_a?(T::Types::Anything)
        T::Types::TypedClass::Anything::Private::INSTANCE
      else
        T::Types::TypedClass::Private::Pool.type_for_module(type)
      end
    end
  end

  module Module
    def self.[](type)
      if type.is_a?(T::Types::Untyped)
        T::Types::TypedModule::Untyped::Private::INSTANCE
      elsif type.is_a?(T::Types::Anything)
        T::Types::TypedModule::Anything::Private::INSTANCE
      else
        T::Types::TypedModule::Private::Pool.type_for_module(type)
      end
    end
  end
end
