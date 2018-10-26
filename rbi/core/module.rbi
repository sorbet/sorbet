# typed: true
class Module < Object
  ARGF = T.let(T.unsafe(nil), Object)
  ARGV = T.let(T.unsafe(nil), Array)
  CROSS_COMPILING = T.let(T.unsafe(nil), NilClass)
  FALSE = T.let(T.unsafe(nil), FalseClass)
  NIL = T.let(T.unsafe(nil), NilClass)
  RUBY_COPYRIGHT = T.let(T.unsafe(nil), String)
  RUBY_DESCRIPTION = T.let(T.unsafe(nil), String)
  RUBY_ENGINE = T.let(T.unsafe(nil), String)
  RUBY_ENGINE_VERSION = T.let(T.unsafe(nil), String)
  RUBY_PATCHLEVEL = T.let(T.unsafe(nil), Integer)
  RUBY_PLATFORM = T.let(T.unsafe(nil), String)
  RUBY_RELEASE_DATE = T.let(T.unsafe(nil), String)
  RUBY_REVISION = T.let(T.unsafe(nil), Integer)
  RUBY_VERSION = T.let(T.unsafe(nil), String)
  STDERR = T.let(T.unsafe(nil), IO)
  STDIN = T.let(T.unsafe(nil), IO)
  STDOUT = T.let(T.unsafe(nil), IO)
  TOPLEVEL_BINDING = T.let(T.unsafe(nil), Binding)
  TRUE = T.let(T.unsafe(nil), TrueClass)

  sig {returns(T::Array[Integer])}
  def self.constants(); end

  sig {returns(T::Array[Module])}
  def self.nesting(); end

  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T.any(TrueClass, FalseClass)))
  end
  def <(other); end

  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T.any(TrueClass, FalseClass)))
  end
  def <=(other); end

  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ==(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ===(other); end

  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T.any(TrueClass, FalseClass)))
  end
  def >(other); end

  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T.any(TrueClass, FalseClass)))
  end
  def >=(other); end

  sig do
    params(
        new_name: Symbol,
        old_name: Symbol,
    )
    .returns(T.self_type)
  end
  def alias_method(new_name, old_name); end

  sig {returns(T::Array[Module])}
  def ancestors(); end

  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def append_features(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(NilClass)
  end
  def attr_accessor(*arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(NilClass)
  end
  def attr_reader(*arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(NilClass)
  end
  def attr_writer(*arg0); end

  sig do
    params(
        _module: Symbol,
        filename: String,
    )
    .returns(NilClass)
  end
  def autoload(_module, filename); end

  sig do
    params(
        name: Symbol,
    )
    .returns(T.nilable(String))
  end
  def autoload?(name); end

  sig do
    params(
        arg0: String,
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  def class_eval(arg0, filename=T.unsafe(nil), lineno=T.unsafe(nil)); end

  sig do
    params(
        args: BasicObject,
        blk: BasicObject,
    )
    .returns(T.untyped)
  end
  def class_exec(*args, &blk); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def class_variable_defined?(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.untyped)
  end
  def class_variable_get(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: BasicObject,
    )
    .returns(T.untyped)
  end
  def class_variable_set(arg0, arg1); end

  sig do
    params(
        inherit: T.any(TrueClass, FalseClass),
    )
    .returns(T::Array[Symbol])
  end
  def class_variables(inherit=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        inherit: T.any(TrueClass, FalseClass),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def const_defined?(arg0, inherit=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        inherit: T.any(TrueClass, FalseClass),
    )
    .returns(T.untyped)
  end
  def const_get(arg0, inherit=T.unsafe(nil)); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  def const_missing(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: BasicObject,
    )
    .returns(T.untyped)
  end
  def const_set(arg0, arg1); end

  sig do
    params(
        inherit: T.any(TrueClass, FalseClass),
    )
    .returns(T::Array[Symbol])
  end
  def constants(inherit=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: Method,
    )
    .returns(Symbol)
  end
  sig do
    params(
        arg0: T.any(Symbol, String),
        blk: BasicObject,
    )
    .returns(Symbol)
  end
  def define_method(arg0, arg1=T.unsafe(nil), &blk); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def eql?(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def equal?(other); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.untyped)
  end
  def extend_object(arg0); end

  sig do
    params(
        othermod: Module,
    )
    .returns(T.untyped)
  end
  def extended(othermod); end

  sig {returns(T.self_type)}
  def freeze(); end

  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def include(*arg0); end

  sig do
    params(
        arg0: Module,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def include?(arg0); end

  sig do
    params(
        othermod: Module,
    )
    .returns(T.untyped)
  end
  def included(othermod); end

  sig {returns(T::Array[Module])}
  def included_modules(); end

  sig {returns(Object)}
  sig do
    params(
        blk: T.proc.params(arg0: Module).returns(BasicObject),
    )
    .void
  end
  def initialize(&blk); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(UnboundMethod)
  end
  def instance_method(arg0); end

  sig do
    params(
        include_super: T.any(TrueClass, FalseClass),
    )
    .returns(T::Array[Symbol])
  end
  def instance_methods(include_super=T.unsafe(nil)); end

  sig do
    params(
        meth: Symbol,
    )
    .returns(T.untyped)
  end
  def method_added(meth); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def method_defined?(arg0); end

  sig do
    params(
        method_name: Symbol,
    )
    .returns(T.untyped)
  end
  def method_removed(method_name); end

  sig do
    params(
        arg0: String,
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  def module_eval(arg0, filename=T.unsafe(nil), lineno=T.unsafe(nil)); end

  sig do
    params(
        args: BasicObject,
        blk: BasicObject,
    )
    .returns(T.untyped)
  end
  def module_exec(*args, &blk); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def module_function(*arg0); end

  sig {returns(String)}
  def name(); end

  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def prepend(*arg0); end

  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def prepend_features(arg0); end

  sig do
    params(
        othermod: Module,
    )
    .returns(T.untyped)
  end
  def prepended(othermod); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def private(*arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def private_class_method(*arg0); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.self_type)
  end
  def private_constant(*arg0); end

  sig do
    params(
        include_super: T.any(TrueClass, FalseClass),
    )
    .returns(T::Array[Symbol])
  end
  def private_instance_methods(include_super=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def private_method_defined?(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def protected(*arg0); end

  sig do
    params(
        include_super: T.any(TrueClass, FalseClass),
    )
    .returns(T::Array[Symbol])
  end
  def protected_instance_methods(include_super=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def protected_method_defined?(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def public(*arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def public_class_method(*arg0); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.self_type)
  end
  def public_constant(*arg0); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(UnboundMethod)
  end
  def public_instance_method(arg0); end

  sig do
    params(
        include_super: T.any(TrueClass, FalseClass),
    )
    .returns(T::Array[Symbol])
  end
  def public_instance_methods(include_super=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def public_method_defined?(arg0); end

  sig do
    params(
        arg0: Class,
        blk: T.proc.params(arg0: T.untyped).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  def refine(arg0, &blk); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  def remove_class_variable(arg0); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  def remove_const(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def remove_method(arg0); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def singleton_class?(); end

  sig {returns(String)}
  def to_s(); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def undef_method(arg0); end

  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def using(arg0); end

  sig {returns(String)}
  def inspect(); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(NilClass)
  end
  def attr(*arg0); end
end
