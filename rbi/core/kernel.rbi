# typed: true
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

  Sorbet.sig(
      start_or_range: Integer,
      length: Integer,
  )
  .returns(T.nilable(T::Array[String]))
  Sorbet.sig(
      start_or_range: T::Range[Integer],
  )
  .returns(T.nilable(T::Array[String]))
  def caller(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end

  Sorbet.sig(
      start_or_range: Integer,
      length: Integer,
  )
  .returns(T.nilable(T::Array[String]))
  Sorbet.sig(
      start_or_range: T::Range[Integer],
  )
  .returns(T.nilable(T::Array[String]))
  def caller_locations(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: String,
      arg1: Binding,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  def eval(arg0, arg1=T.unsafe(nil), filename=T.unsafe(nil), lineno=T.unsafe(nil)); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def iterator?(); end

  Sorbet.sig.returns(T::Array[Symbol])
  def local_variables(); end

  Sorbet.sig(
      number: Numeric,
  )
  .returns(Numeric)
  def srand(number); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def !~(other); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(other); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(NilClass)
  def =~(other); end

  Sorbet.sig.returns(T.self_type)
  def clone(); end

  Sorbet.sig(
      port: IO,
  )
  .returns(NilClass)
  def display(port); end

  Sorbet.sig.returns(T.self_type)
  def dup(); end

  Sorbet.sig(
      method: Symbol,
      args: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  Sorbet.sig(
      method: Symbol,
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  def enum_for(method=T.unsafe(nil), *args, &blk); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end

  Sorbet.sig(
      mod: Module,
  )
  .returns(NilClass)
  def extend(mod); end

  Sorbet.sig.returns(T.self_type)
  def freeze(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def frozen?(); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig(
      arg0: Class,
  )
  .returns(T.any(TrueClass, FalseClass))
  def instance_of?(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def instance_variable_defined?(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.untyped)
  def instance_variable_get(arg0); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def instance_variable_set(arg0, arg1); end

  Sorbet.sig.returns(T::Array[Symbol])
  def instance_variables(); end

  Sorbet.sig(
      arg0: T.any(Class, Module),
  )
  .returns(T.any(TrueClass, FalseClass))
  def is_a?(arg0); end

  Sorbet.sig(
      arg0: Class,
  )
  .returns(T.any(TrueClass, FalseClass))
  def kind_of?(arg0); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(Method)
  def method(arg0); end

  Sorbet.sig(
      regular: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def methods(regular=T.unsafe(nil)); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def nil?(); end

  Sorbet.sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def private_methods(all=T.unsafe(nil)); end

  Sorbet.sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def protected_methods(all=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(Method)
  def public_method(arg0); end

  Sorbet.sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def public_methods(all=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: T.any(Symbol, String),
      args: BasicObject,
  )
  .returns(T.untyped)
  def public_send(arg0, *args); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  def remove_instance_variable(arg0); end

  Sorbet.sig(
      arg0: T.any(String, Symbol),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  Sorbet.sig(
      arg0: T.any(String, Symbol),
      arg1: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def send(arg0, *arg1, &blk); end

  Sorbet.sig.returns(Class)
  def singleton_class(); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(Method)
  def singleton_method(arg0); end

  Sorbet.sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def singleton_methods(all=T.unsafe(nil)); end

  Sorbet.sig.returns(T.self_type)
  def taint(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def tainted?(); end

  Sorbet.sig(
      method: Symbol,
      args: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  Sorbet.sig(
      method: Symbol,
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  def to_enum(method=T.unsafe(nil), *args, &blk); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig.returns(T.self_type)
  def trust(); end

  Sorbet.sig.returns(T.self_type)
  def untaint(); end

  Sorbet.sig.returns(T.self_type)
  def untrust(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def untrusted?(); end

  type_parameters(:T).sig(
      x: Object,
  )
  .returns(T::Array[T.type_parameter(:T)])
  def Array(x); end

  Sorbet.sig(
      initial: T.any(Integer, Float, Rational, BigDecimal, String),
      digits: Integer,
  )
  .void
  def BigDecimal(initial, digits=0); end

  Sorbet.sig(
      x: T.any(Numeric, String),
      y: T.any(Numeric, String),
  )
  .returns(Complex)
  Sorbet.sig(
      x: String,
  )
  .returns(Complex)
  def Complex(x, y=T.unsafe(nil)); end

  Sorbet.sig(
      x: T.any(Numeric, String),
  )
  .returns(Float)
  def Float(x); end

  type_parameters(:K ,:V).sig(
      x: Object,
  )
  .returns(T::Hash[T.type_parameter(:K), T.type_parameter(:V)])
  def Hash(x); end

  Sorbet.sig(
      arg: T.any(Numeric, String),
      base: Integer,
  )
  .returns(Integer)
  def Integer(arg, base=T.unsafe(nil)); end

  Sorbet.sig(
      x: T.any(Numeric, String),
      y: T.any(Numeric, String),
  )
  .returns(Rational)
  Sorbet.sig(
      x: Object,
  )
  .returns(Rational)
  def Rational(x, y=T.unsafe(nil)); end

  Sorbet.sig(
      x: Object,
  )
  .returns(String)
  def String(x); end

  Sorbet.sig.returns(T.nilable(Symbol))
  def __callee__(); end

  Sorbet.sig.returns(T.nilable(String))
  def __dir__(); end

  Sorbet.sig.returns(T.nilable(Symbol))
  def __method__(); end

  Sorbet.sig(
      arg0: String,
  )
  .returns(String)
  def `(arg0); end

  Sorbet.sig(
      msg: String,
  )
  .returns(T.noreturn)
  def abort(msg=T.unsafe(nil)); end

  Sorbet.sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(Proc)
  def at_exit(&blk); end

  Sorbet.sig(
      _module: T.any(String, Symbol),
      filename: String,
  )
  .returns(NilClass)
  def autoload(_module, filename); end

  Sorbet.sig(
      name: T.any(Symbol, String),
  )
  .returns(T.nilable(String))
  def autoload?(name); end

  Sorbet.sig.returns(Binding)
  def binding(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def block_given?(); end

  Sorbet.sig.returns(T.noreturn)
  Sorbet.sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def exit(status=T.unsafe(nil)); end

  Sorbet.sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def exit!(status); end

  Sorbet.sig.returns(T.noreturn)
  Sorbet.sig(
      arg0: String,
  )
  .returns(T.noreturn)
  Sorbet.sig(
      arg0: Class,
      arg1: T::Array[String],
  )
  .returns(T.noreturn)
  Sorbet.sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def fail(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  Sorbet.sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def format(format, *args); end

  Sorbet.sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def gets(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  Sorbet.sig.returns(T::Array[Symbol])
  def global_variables(); end

  Sorbet.sig(
      filename: String,
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def load(filename, arg0=T.unsafe(nil)); end

  Sorbet.sig(
      blk: T.proc().returns(T.untyped)
  )
  .returns(NilClass)
  Sorbet.sig.returns(Enumerator[T.untyped])
  def loop(&blk); end

  Sorbet.sig(
      name: String,
      rest: T.any(String, Integer),
      block: String,
  )
  .returns(T.nilable(IO))
  def open(name, rest=T.unsafe(nil), block=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: IO,
      arg1: String,
      arg2: BasicObject,
  )
  .returns(NilClass)
  def printf(arg0=T.unsafe(nil), arg1=T.unsafe(nil), *arg2); end

  Sorbet.sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def proc(&blk); end

  Sorbet.sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def lambda(&blk); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def putc(arg0); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def puts(*arg0); end

  Sorbet.sig.returns(Float)
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: T::Range[Integer],
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: T::Range[Float],
  )
  .returns(Float)
  def rand(arg0=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def readline(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(T::Array[String])
  def readlines(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  Sorbet.sig(
      path: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def require(path); end

  Sorbet.sig(
      feature: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def require_relative(feature); end

  Sorbet.sig(
      read: T::Array[IO],
      write: T::Array[IO],
      error: T::Array[IO],
      timeout: Integer,
  )
  .returns(T::Array[String])
  def select(read, write=T.unsafe(nil), error=T.unsafe(nil), timeout=T.unsafe(nil)); end

  Sorbet.sig(
      duration: Numeric,
  )
  .returns(Integer)
  def sleep(duration); end

  Sorbet.sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def sprintf(format, *args); end

  Sorbet.sig(
      num: Integer,
      args: BasicObject,
  )
  .returns(T.untyped)
  def syscall(num, *args); end

  Sorbet.sig(
      cmd: String,
      file1: String,
      file2: String,
  )
  .returns(T.any(TrueClass, FalseClass, Time))
  def test(cmd, file1, file2=T.unsafe(nil)); end

  Sorbet.sig(
      msg: String,
  )
  .returns(NilClass)
  def warn(*msg); end

  Sorbet.sig.returns(T.noreturn)
  Sorbet.sig(
      arg0: String,
  )
  .returns(T.noreturn)
  Sorbet.sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  Sorbet.sig(
      arg0: Exception,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def raise(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end
end
