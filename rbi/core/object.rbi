# typed: true

class BasicObject
  sig {returns(T::Boolean)}
  def !(); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def !=(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end

  sig {returns(Integer)}
  def __id__(); end

  sig do
    params(
        arg0: Symbol,
        arg1: BasicObject,
    )
    .returns(T.untyped)
  end
  def __send__(arg0, *arg1); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def equal?(other); end

  sig do
    params(
        arg0: String,
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  sig do
    type_parameters(:U)
    .params(
        blk: T.proc.bind(T.untyped).params().returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def instance_eval(arg0=T.unsafe(nil), filename=T.unsafe(nil), lineno=T.unsafe(nil), &blk); end

  sig do
    type_parameters(:U, :V)
    .params(
        args: T.type_parameter(:V),
        blk: T.proc.bind(T.untyped).params(args: T.untyped).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def instance_exec(*args, &blk); end
end

module Kernel
  RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)

  # A note on global functions:
  #
  # Ruby tends to define global (e.g. "require", "puts") as
  # module_function's on Kernel. In practice, this is mostly
  # superfluous; since Kernel is itself an ancestor of
  # Kernel.singleton_class, all of Kernel's instance methods will
  # automatically appear on Kernel directly. For this reason, we omit
  # module_function and just define these methods as instance methods,
  # for clarity and simplicity.

  sig do
    params(
        start_or_range: Integer,
        length: Integer,
    )
    .returns(T.nilable(T::Array[String]))
  end
  sig do
    params(
        start_or_range: T::Range[Integer],
    )
    .returns(T.nilable(T::Array[String]))
  end
  sig {params.returns(T::Array[String])}
  def caller(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end

  sig do
    params(
        start_or_range: Integer,
        length: Integer,
    )
    .returns(T.nilable(T::Array[Thread::Backtrace::Location]))
  end
  sig do
    params(
        start_or_range: T::Range[Integer],
    )
    .returns(T.nilable(T::Array[Thread::Backtrace::Location]))
  end
  def caller_locations(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end

  sig do
    params(
        tag: Object,
        blk: T.proc.params(arg0: Object).returns(BasicObject),
    )
    .returns(BasicObject)
  end
  def catch(tag=Object.new, &blk); end

  sig do
    params(
        symbol: T.any(Symbol, String),
        method: T.any(Proc, Method, UnboundMethod)
    )
    .returns(Symbol)
  end
  sig do
    params(
        symbol: T.any(Symbol, String),
        blk: BasicObject
    )
    .returns(Symbol)
  end
  def define_singleton_method(symbol, method=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: String,
        arg1: Binding,
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  def eval(arg0, arg1=T.unsafe(nil), filename=T.unsafe(nil), lineno=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  def iterator?(); end

  sig {returns(T::Array[Symbol])}
  def local_variables(); end

  sig do
    params(
        number: Numeric,
    )
    .returns(Numeric)
  end
  def srand(number); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def !~(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(NilClass)
  end
  def =~(other); end

  sig {returns(T.self_type)}
  def clone(); end

  sig do
    params(
        port: IO,
    )
    .returns(NilClass)
  end
  def display(port); end

  sig {returns(T.self_type)}
  def dup(); end

  sig do
    params(
        method: Symbol,
        args: BasicObject,
    )
    .returns(Enumerator[T.untyped])
  end
  sig do
    params(
        method: Symbol,
        args: BasicObject,
        blk: BasicObject,
    )
    .returns(Enumerator[T.untyped])
  end
  def enum_for(method=T.unsafe(nil), *args, &blk); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(other); end

  sig do
    params(
        mod: Module,
    )
    .returns(NilClass)
  end
  def extend(mod); end

  sig {returns(T.self_type)}
  def freeze(); end

  sig {returns(T::Boolean)}
  def frozen?(); end

  sig {returns(Integer)}
  def hash(); end

  sig {params(object: T.self_type).returns(T.self_type)}
  def initialize_copy(object); end

  sig {returns(String)}
  def inspect(); end

  sig do
    params(
        arg0: Class,
    )
    .returns(T::Boolean)
  end
  def instance_of?(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T::Boolean)
  end
  def instance_variable_defined?(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.untyped)
  end
  def instance_variable_get(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: BasicObject,
    )
    .returns(T.untyped)
  end
  def instance_variable_set(arg0, arg1); end

  sig {returns(T::Array[Symbol])}
  def instance_variables(); end

  sig do
    params(
        arg0: T.any(Class, Module),
    )
    .returns(T::Boolean)
  end
  def is_a?(arg0); end

  sig do
    params(
        arg0: Class,
    )
    .returns(T::Boolean)
  end
  def kind_of?(arg0); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(Method)
  end
  def method(arg0); end

  sig do
    params(
        regular: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def methods(regular=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  def nil?(); end

  sig do
    params(
        all: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def private_methods(all=T.unsafe(nil)); end

  sig do
    params(
        all: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def protected_methods(all=T.unsafe(nil)); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(Method)
  end
  def public_method(arg0); end

  sig do
    params(
        all: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def public_methods(all=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        args: BasicObject,
    )
    .returns(T.untyped)
  end
  def public_send(arg0, *args); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  def remove_instance_variable(arg0); end

  sig do
    params(
        arg0: T.any(String, Symbol),
        arg1: BasicObject,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        arg0: T.any(String, Symbol),
        arg1: BasicObject,
        blk: BasicObject,
    )
    .returns(T.untyped)
  end
  def send(arg0, *arg1, &blk); end

  sig {returns(Class)}
  def singleton_class(); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(Method)
  end
  def singleton_method(arg0); end

  sig do
    params(
        all: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def singleton_methods(all=T.unsafe(nil)); end

  sig {returns(T.self_type)}
  def taint(); end

  sig {returns(T::Boolean)}
  def tainted?(); end

  sig do
    params(
      blk: T.proc.params(x: T.untyped).void
    )
    .returns(T.self_type)
  end
  def tap(&blk); end

  sig do
    params(
        method: Symbol,
        args: BasicObject,
    )
    .returns(Enumerator[T.untyped])
  end
  sig do
    params(
        method: Symbol,
        args: BasicObject,
        blk: BasicObject,
    )
    .returns(Enumerator[T.untyped])
  end
  def to_enum(method=T.unsafe(nil), *args, &blk); end

  sig {returns(String)}
  def to_s(); end

  sig {returns(T.self_type)}
  def trust(); end

  sig do
    params(
        arg: BasicObject,
    )
    .void
  end
  def undef(*arg); end

  sig {returns(T.self_type)}
  def untaint(); end

  sig {returns(T.self_type)}
  def untrust(); end

  sig {returns(T::Boolean)}
  def untrusted?(); end

  sig {params(x: NilClass).returns([])}
  sig do
    params(
        x: BasicObject,
    )
    .returns(T::Array[T.untyped])
  end
  def Array(x); end

  sig do
    params(
        initial: T.any(Integer, Float, Rational, BigDecimal, String),
        digits: Integer,
    )
    .returns(BigDecimal)
  end
  def BigDecimal(initial, digits=0); end

  sig do
    params(
        x: T.any(Numeric, String),
        y: T.any(Numeric, String),
    )
    .returns(Complex)
  end
  sig do
    params(
        x: String,
    )
    .returns(Complex)
  end
  def Complex(x, y=T.unsafe(nil)); end

  sig do
    params(
        x: T.any(Numeric, String),
    )
    .returns(Float)
  end
  def Float(x); end

  sig do
    type_parameters(:K ,:V).params(
        x: Object,
    )
    .returns(T::Hash[T.type_parameter(:K), T.type_parameter(:V)])
  end
  def Hash(x); end

  sig do
    params(
        arg: T.any(Numeric, String),
        base: Integer,
    )
    .returns(Integer)
  end
  def Integer(arg, base=T.unsafe(nil)); end

  sig do
    params(
        x: T.any(Numeric, String),
        y: T.any(Numeric, String),
    )
    .returns(Rational)
  end
  sig do
    params(
        x: Object,
    )
    .returns(Rational)
  end
  def Rational(x, y=T.unsafe(nil)); end

  sig do
    params(
        x: Object,
    )
    .returns(String)
  end
  def String(x); end

  sig {returns(T.nilable(Symbol))}
  def __callee__(); end

  sig {returns(T.nilable(String))}
  def __dir__(); end

  sig {returns(T.nilable(Symbol))}
  def __method__(); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def `(arg0); end

  sig do
    params(
        msg: String,
    )
    .returns(T.noreturn)
  end
  def abort(msg=T.unsafe(nil)); end

  sig do
    params(
        blk: T.proc.params.returns(BasicObject),
    )
    .returns(Proc)
  end
  def at_exit(&blk); end

  sig do
    params(
        _module: T.any(String, Symbol),
        filename: String,
    )
    .returns(NilClass)
  end
  def autoload(_module, filename); end

  sig do
    params(
        name: T.any(Symbol, String),
    )
    .returns(T.nilable(String))
  end
  def autoload?(name); end

  sig {returns(Binding)}
  def binding(); end

  sig {returns(T::Boolean)}
  def block_given?(); end

  sig {returns(T.noreturn)}
  sig do
    params(
        status: T.any(Integer, TrueClass, FalseClass),
    )
    .returns(T.noreturn)
  end
  def exit(status=T.unsafe(nil)); end

  sig do
    params(
        status: T.any(Integer, TrueClass, FalseClass),
    )
    .returns(T.noreturn)
  end
  def exit!(status); end

  sig {returns(T.noreturn)}
  sig do
    params(
        arg0: String,
    )
    .returns(T.noreturn)
  end
  sig do
    params(
        arg0: Class,
        arg1: T::Array[String],
    )
    .returns(T.noreturn)
  end
  sig do
    params(
        arg0: Class,
        arg1: String,
        arg2: T::Array[String],
    )
    .returns(T.noreturn)
  end
  def fail(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  sig do
    params(
        format: String,
        args: BasicObject,
    )
    .returns(String)
  end
  def format(format, *args); end

  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(String)
  end
  def gets(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  sig {returns(T::Array[Symbol])}
  def global_variables(); end

  sig do
    params(
        filename: String,
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def load(filename, arg0=T.unsafe(nil)); end

  sig {params(blk: T.proc.params.returns(T.untyped)).returns(T.untyped)}
  sig {returns(Enumerator[T.untyped])}
  def loop(&blk); end

  sig do
    params(
        name: String,
        rest: T.any(String, Integer),
        block: String,
    )
    .returns(T.nilable(IO))
  end
  def open(name, rest=T.unsafe(nil), block=T.unsafe(nil)); end

  sig {params(args: Kernel).returns(NilClass)}
  def print(*args); end

  sig do
    params(
        arg0: IO,
        arg1: String,
        arg2: BasicObject,
    )
    .returns(NilClass)
  end
  def printf(arg0=T.unsafe(nil), arg1=T.unsafe(nil), *arg2); end

  sig do
    params(
        blk: BasicObject,
    )
    .returns(Proc)
  end
  def proc(&blk); end

  sig do
    params(
        blk: BasicObject,
    )
    .returns(Proc)
  end
  def lambda(&blk); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def putc(arg0); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def puts(*arg0); end

  sig {returns(Float)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: T::Range[Float],
    )
    .returns(Float)
  end
  def rand(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(String)
  end
  def readline(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(T::Array[String])
  end
  def readlines(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  sig do
    params(
        path: String,
    )
    .returns(T::Boolean)
  end
  def require(path); end

  sig do
    params(
        feature: String,
    )
    .returns(T::Boolean)
  end
  def require_relative(feature); end

  sig do
    params(
        read: T::Array[IO],
        write: T::Array[IO],
        error: T::Array[IO],
        timeout: Integer,
    )
    .returns(T::Array[String])
  end
  def select(read, write=T.unsafe(nil), error=T.unsafe(nil), timeout=T.unsafe(nil)); end

  sig do
    params(
        duration: Numeric,
    )
    .returns(Integer)
  end
  def sleep(duration); end

  sig do
    params(
        format: String,
        args: BasicObject,
    )
    .returns(String)
  end
  def sprintf(format, *args); end

  sig do
    params(
        num: Integer,
        args: BasicObject,
    )
    .returns(T.untyped)
  end
  def syscall(num, *args); end

  sig do
    params(
        cmd: String,
        file1: String,
        file2: String,
    )
    .returns(T.any(TrueClass, FalseClass, Time))
  end
  def test(cmd, file1, file2=T.unsafe(nil)); end

  sig do
    params(
        tag: Object,
        obj: BasicObject,
    )
    .returns(T.noreturn)
  end
  def throw(tag, obj=nil); end

  sig do
    params(
        msg: String,
    )
    .returns(NilClass)
  end
  def warn(*msg); end

  sig {returns(T.noreturn)}
  sig do
    params(
        arg0: String,
    )
    .returns(T.noreturn)
  end
  sig do
    params(
        arg0: Class,
        arg1: String,
        arg2: T::Array[String],
    )
    .returns(T.noreturn)
  end
  sig do
    params(
        arg0: Exception,
        arg1: String,
        arg2: T::Array[String],
    )
    .returns(T.noreturn)
  end
  def raise(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  sig { params(args: String).returns(T.noreturn) }
  def exec(*args); end

  sig { params(args: String).returns(T.any(NilClass, FalseClass, TrueClass)) }
  def system(*args); end
end

class Object < BasicObject
  include Kernel

  sig {returns(Integer)}
  def object_id(); end
end

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
    .returns(T.nilable(T::Boolean))
  end
  def <(other); end

  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T::Boolean))
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
    .returns(T::Boolean)
  end
  def ==(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(other); end

  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T::Boolean))
  end
  def >(other); end

  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T::Boolean))
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
    .returns(T::Boolean)
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
        inherit: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def class_variables(inherit=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        inherit: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def const_defined?(arg0, inherit=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        inherit: T::Boolean,
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
        inherit: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def constants(inherit=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: T.any(Proc, Method, UnboundMethod)
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
    .returns(T::Boolean)
  end
  def eql?(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
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
    .returns(T::Boolean)
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
        include_super: T::Boolean,
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
    .returns(T::Boolean)
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
        include_super: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def private_instance_methods(include_super=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T::Boolean)
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
        include_super: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def protected_instance_methods(include_super=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T::Boolean)
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
        include_super: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def public_instance_methods(include_super=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T::Boolean)
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

  sig {returns(T::Boolean)}
  def singleton_class?(); end

  sig {returns(String)}
  def to_s(); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def undefMethod(arg0); end

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

class Class < Module
  sig {returns(T.untyped)}
  def allocate(); end

  sig {params(args: T.untyped).returns(T.untyped)}
  def new(*args); end

  sig {params(superClass: NilClass).returns(Class)}
  sig {params(superClass: NilClass, blk: T.proc.bind(Class).params(arg0: Class).void).returns(Class)}
  sig {type_parameters(:T).params(superClass: T.type_parameter(:T)).returns(T.type_parameter(:T))}
  sig {type_parameters(:T).params(superClass: T.type_parameter(:T), blk: T.proc.bind(Class).params(arg0: Class).void).returns(T.type_parameter(:T))}
  def self.new(superClass = nil, &blk)
    T.unsafe(nil)
  end

  sig do
    params(
        arg0: Class,
    )
    .returns(T.untyped)
  end
  def inherited(arg0); end

  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def instance_methods(arg0=T.unsafe(nil)); end

  sig {returns(T.nilable(String))}
  def name(); end

  sig {returns(T.nilable(Class))}
  sig {returns(Class)}
  def superclass(); end

  sig {void}
  sig do
    params(
        superclass: Class,
    )
    .void
  end
  sig do
    params(
        blk: T.proc.params(arg0: Class).returns(BasicObject),
    )
    .void
  end
  sig do
    params(
        superclass: Class,
        blk: T.proc.params(arg0: Class).returns(BasicObject),
    )
    .void
  end
  def initialize(superclass=_, &blk); end
end

class Data < Object
end

class NilClass < Object
  sig do
    params(
        obj: BasicObject,
    )
    .returns(FalseClass)
  end
  def &(obj); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ^(obj); end

  sig {returns(Rational)}
  def rationalize(); end

  sig {returns([])}
  def to_a(); end

  sig {returns(Complex)}
  def to_c(); end

  sig {returns(Float)}
  def to_f(); end

  sig {returns(T::Hash[T.untyped, T.untyped])}
  def to_h(); end

  sig {returns(Rational)}
  def to_r(); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def |(obj); end

  sig {returns(TrueClass)}
  def nil?; end
end

class TrueClass
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(TrueClass)}
  def |(obj)
  end
  sig {returns(FalseClass)}
  def !
  end
end

class FalseClass
  sig {params(obj: BasicObject).returns(FalseClass)}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def |(obj)
  end
  sig {returns(TrueClass)}
  def !
  end
end
