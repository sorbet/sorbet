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

  Sorbet.sig.returns(T::Array[Integer])
  def self.constants(); end

  Sorbet.sig.returns(T::Array[Module])
  def self.nesting(); end

  Sorbet.sig(
      other: Module,
  )
  .returns(T.nilable(T.any(TrueClass, FalseClass)))
  def <(other); end

  Sorbet.sig(
      other: Module,
  )
  .returns(T.nilable(T.any(TrueClass, FalseClass)))
  def <=(other); end

  Sorbet.sig(
      other: Module,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(other); end

  Sorbet.sig(
      other: Module,
  )
  .returns(T.nilable(T.any(TrueClass, FalseClass)))
  def >(other); end

  Sorbet.sig(
      other: Module,
  )
  .returns(T.nilable(T.any(TrueClass, FalseClass)))
  def >=(other); end

  Sorbet.sig(
      new_name: Symbol,
      old_name: Symbol,
  )
  .returns(T.self_type)
  def alias_method(new_name, old_name); end

  Sorbet.sig.returns(T::Array[Module])
  def ancestors(); end

  Sorbet.sig(
      arg0: Module,
  )
  .returns(T.self_type)
  def append_features(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(NilClass)
  def attr_accessor(*arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(NilClass)
  def attr_reader(*arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(NilClass)
  def attr_writer(*arg0); end

  Sorbet.sig(
      _module: Symbol,
      filename: String,
  )
  .returns(NilClass)
  def autoload(_module, filename); end

  Sorbet.sig(
      name: Symbol,
  )
  .returns(T.nilable(String))
  def autoload?(name); end

  Sorbet.sig(
      arg0: String,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  def class_eval(arg0, filename=T.unsafe(nil), lineno=T.unsafe(nil)); end

  Sorbet.sig(
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def class_exec(*args, &blk); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def class_variable_defined?(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.untyped)
  def class_variable_get(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def class_variable_set(arg0, arg1); end

  Sorbet.sig(
      inherit: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def class_variables(inherit=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
      inherit: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def const_defined?(arg0, inherit=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
      inherit: T.any(TrueClass, FalseClass),
  )
  .returns(T.untyped)
  def const_get(arg0, inherit=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  def const_missing(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def const_set(arg0, arg1); end

  Sorbet.sig(
      inherit: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def constants(inherit=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: Symbol,
      arg1: Method,
  )
  .returns(Symbol)
  Sorbet.sig(
      arg0: Symbol,
      blk: BasicObject,
  )
  .returns(Symbol)
  def define_method(arg0, arg1=T.unsafe(nil), &blk); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(other); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.untyped)
  def extend_object(arg0); end

  Sorbet.sig(
      othermod: Module,
  )
  .returns(T.untyped)
  def extended(othermod); end

  Sorbet.sig.returns(T.self_type)
  def freeze(); end

  Sorbet.sig(
      arg0: Module,
  )
  .returns(T.self_type)
  def include(*arg0); end

  Sorbet.sig(
      arg0: Module,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(arg0); end

  Sorbet.sig(
      othermod: Module,
  )
  .returns(T.untyped)
  def included(othermod); end

  Sorbet.sig.returns(T::Array[Module])
  def included_modules(); end

  Sorbet.sig.returns(Object)
  Sorbet.sig(
      blk: T.proc(arg0: Module).returns(BasicObject),
  )
  .void
  def initialize(&blk); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(UnboundMethod)
  def instance_method(arg0); end

  Sorbet.sig(
      include_super: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def instance_methods(include_super=T.unsafe(nil)); end

  Sorbet.sig(
      meth: Symbol,
  )
  .returns(T.untyped)
  def method_added(meth); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def method_defined?(arg0); end

  Sorbet.sig(
      method_name: Symbol,
  )
  .returns(T.untyped)
  def method_removed(method_name); end

  Sorbet.sig(
      arg0: String,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  def module_eval(arg0, filename=T.unsafe(nil), lineno=T.unsafe(nil)); end

  Sorbet.sig(
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def module_exec(*args, &blk); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.self_type)
  def module_function(*arg0); end

  Sorbet.sig.returns(String)
  def name(); end

  Sorbet.sig(
      arg0: Module,
  )
  .returns(T.self_type)
  def prepend(*arg0); end

  Sorbet.sig(
      arg0: Module,
  )
  .returns(T.self_type)
  def prepend_features(arg0); end

  Sorbet.sig(
      othermod: Module,
  )
  .returns(T.untyped)
  def prepended(othermod); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.self_type)
  def private(*arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.self_type)
  def private_class_method(*arg0); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(T.self_type)
  def private_constant(*arg0); end

  Sorbet.sig(
      include_super: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def private_instance_methods(include_super=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def private_method_defined?(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.self_type)
  def protected(*arg0); end

  Sorbet.sig(
      include_super: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def protected_instance_methods(include_super=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def protected_method_defined?(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.self_type)
  def public(*arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.self_type)
  def public_class_method(*arg0); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(T.self_type)
  def public_constant(*arg0); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(UnboundMethod)
  def public_instance_method(arg0); end

  Sorbet.sig(
      include_super: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def public_instance_methods(include_super=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def public_method_defined?(arg0); end

  Sorbet.sig(
      arg0: Class,
      blk: T.proc(arg0: T.untyped).returns(BasicObject),
  )
  .returns(T.self_type)
  def refine(arg0, &blk); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  def remove_class_variable(arg0); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  def remove_const(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.self_type)
  def remove_method(arg0); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def singleton_class?(); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.self_type)
  def undef_method(arg0); end

  Sorbet.sig(
      arg0: Module,
  )
  .returns(T.self_type)
  def using(arg0); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(NilClass)
  def attr(*arg0); end
end
