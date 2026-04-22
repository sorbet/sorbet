# typed: true

class T::Private::Methods::Signature
  sig {returns(UnboundMethod)}
  attr_reader :method
  sig {returns(Symbol)}
  attr_reader :method_name
  sig {returns(T::Array[[Symbol, T::Types::Base]])}
  attr_reader :arg_types
  sig {returns(T::Hash[Symbol, T::Types::Base])}
  attr_reader :kwarg_types
  sig {returns(T.nilable(T::Types::Base))}
  attr_reader :block_type
  sig {returns(T.nilable(Symbol))}
  attr_reader :block_name
  sig {returns(T.nilable(T::Types::Base))}
  attr_reader :rest_type
  sig {returns(T.nilable(Symbol))}
  attr_reader :rest_name
  sig {returns(T.nilable(T::Types::Base))}
  attr_reader :keyrest_type
  sig {returns(T.nilable(Symbol))}
  attr_reader :keyrest_name
  sig {returns(T.nilable(T::Types::Base))}
  attr_reader :bind
  sig {returns(T::Types::Base)}
  attr_reader :effective_return_type
  sig {returns(T::Types::Base)}
  attr_reader :return_type
  sig {returns(String)}
  attr_reader :mode
  sig {returns(Integer)}
  attr_reader :req_arg_count
  sig {returns(T::Array[Symbol])}
  attr_reader :req_kwarg_names
  sig {returns(Symbol)}
  attr_reader :check_level
  sig {returns(T::Array[[Symbol, Symbol]])}
  attr_reader :parameters
  sig {returns(T.nilable(T::Array[T.untyped]))}
  attr_reader :on_failure
  sig {returns(T.nilable(T.any(TrueClass, FalseClass, Symbol)))}
  attr_reader :override_allow_incompatible
  sig {returns(T::Boolean)}
  attr_reader :defined_raw

  sig do
    params(
      method: UnboundMethod,
      mode: String,
      # This is a lie: it would be better to say T.any([Symbol], [Symbol, Symbol])
      # but Sorbet can't lub unequal tuples rught now (leads to T::Array[T.untyped])
      parameters: T::Array[[Symbol, Symbol]]
    )
      .returns(T.attached_class)
  end
  def self.new_untyped(method:, mode: T::Private::Methods::Modes.untyped, parameters: method.parameters); end

  sig do
    params(
      method: UnboundMethod,
      method_name: Symbol,
      raw_arg_types: T::Hash[T.nilable(Symbol), T.untyped],
      raw_return_type: T.untyped,
      bind: T.untyped,
      mode: String,
      check_level: T.nilable(Symbol),
      on_failure: T.nilable(T::Array[T.untyped]),
      parameters: T::Array[[Symbol, Symbol]],
      override_allow_incompatible: T.any(TrueClass, FalseClass, Symbol),
      defined_raw: T::Boolean,
    ).void
  end
  def initialize(method:, method_name:, raw_arg_types:, raw_return_type:, bind:, mode:, check_level:, on_failure:, parameters: method.parameters, override_allow_incompatible: false, defined_raw: false)
    @method = T.let(method, UnboundMethod)
    @method_name = T.let(method_name, Symbol)
    @block_type = T.let(nil, T.nilable(T::Types::Base))
    @block_name = T.let(nil, T.nilable(Symbol))
    @rest_type = T.let(nil, T.nilable(T::Types::Base))
    @rest_name = T.let(nil, T.nilable(Symbol))
    @keyrest_type = T.let(nil, T.nilable(T::Types::Base))
    @keyrest_name = T.let(nil, T.nilable(Symbol))
    @return_type = T.let(raw_return_type, T::Types::Base)
    @effective_return_type = T.let(raw_return_type, T::Types::Base)
    @bind = T.let(nil, T.nilable(T::Types::Base))
    @mode = T.let(mode, String)
    @check_level = T.let(check_level, Symbol)
    @parameters = T.let(parameters, T::Array[[Symbol, Symbol]])
    @on_failure = T.let(on_failure, T.nilable(T::Array[T.untyped]))
    @override_allow_incompatible = T.let(override_allow_incompatible, T.any(T::Boolean, Symbol))
    @defined_raw = T.let(defined_raw, T::Boolean)
    @arg_types = T.let([], T::Array[[Symbol, T::Types::Base]])
    @kwarg_types = T.let([], T::Hash[Symbol, T::Types::Base])
    @req_arg_count = T.let(0, Integer)
    @req_kwarg_names = T.let([], T::Array[Symbol])
  end

  sig { params(method_name: Symbol).returns(Symbol) }
  attr_writer :method_name

  sig {params(alias_name: Symbol).returns(T::Private::Methods::Signature)}
  def as_alias(alias_name); end

  sig {returns(Integer)}
  def arg_count; end

  sig {returns(T::Array[Symbol])}
  def kwarg_names; end

  sig {returns(Module)}
  def owner; end

  sig {returns(String)}
  def dsl_method; end

  sig {params(args: T::Array[Kernel], blk: T.proc.params(name: T.untyped, val: T.untyped, type: T::Types::Base).void).void}
  def each_args_value_type(args, &blk); end

  sig {returns(String)}
  def method_desc; end

  sig {void}
  def force_type_init; end

  EMPTY_LIST = T.let(T.unsafe(nil), T::Array[T.untyped])
  EMPTY_HASH = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
end
