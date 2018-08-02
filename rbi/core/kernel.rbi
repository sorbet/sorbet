# typed: true
module Kernel
  RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)

  type_parameters(:T).sig(
      x: Object,
  )
  .returns(T::Array[T.type_parameter(:T)])
  def self.Array(x); end
  
  sig(
      initial: T.any(Integer, Float, Rational, BigDecimal, String),
      digits: Integer,
  )
  .void
  def self.BigDecimal(initial, digits=0); end

  sig(
      x: T.any(Numeric, String),
      y: T.any(Numeric, String),
  )
  .returns(Complex)
  sig(
      x: String,
  )
  .returns(Complex)
  def self.Complex(x, y=_); end

  sig(
      x: T.any(Numeric, String),
  )
  .returns(Float)
  def self.Float(x); end

  type_parameters(:K ,:V).sig(
      x: Object,
  )
  .returns(T::Hash[T.type_parameter(:K), T.type_parameter(:V)])
  def self.Hash(x); end

  sig(
      arg: T.any(Numeric, String),
      base: Integer,
  )
  .returns(Integer)
  def self.Integer(arg, base=_); end

  sig(
      x: T.any(Numeric, String),
      y: T.any(Numeric, String)
  )
  .returns(Rational)
  sig(
      x: Object,
  )
  .returns(Rational)
  def self.Rational(x, y=_); end

  sig(
      x: Object,
  )
  .returns(String)
  def self.String(x); end

  sig.returns(T.nilable(Symbol))
  def self.__callee__(); end

  sig.returns(T.nilable(String))
  def self.__dir__(); end

  sig.returns(T.nilable(Symbol))
  def self.__method__(); end

  sig(
      arg0: String,
  )
  .returns(String)
  def self.`(arg0); end

  sig(
      msg: String,
  )
  .returns(T.noreturn)
  def self.abort(msg=_); end

  sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(Proc)
  def self.at_exit(&blk); end

  sig(
      _module: T.any(String, Symbol),
      filename: String,
  )
  .returns(NilClass)
  def self.autoload(_module, filename); end

  sig(
      name: T.any(Symbol, String),
  )
  .returns(T.nilable(String))
  def self.autoload?(name); end

  sig.returns(Binding)
  def self.binding(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.block_given?(); end

  sig(
      start_or_range: Integer,
      length: Integer,
  )
  .returns(T.nilable(T::Array[String]))
  sig(
      start_or_range: T::Range[Integer],
  )
  .returns(T.nilable(T::Array[String]))
  def self.caller(start_or_range=_, length=_); end

  sig(
      start_or_range: Integer,
      length: Integer,
  )
  .returns(T.nilable(T::Array[String]))
  sig(
      start_or_range: T::Range[Integer],
  )
  .returns(T.nilable(T::Array[String]))
  def self.caller_locations(start_or_range=_, length=_); end

  sig(
      arg0: String,
      arg1: Binding,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  def self.eval(arg0, arg1=_, filename=_, lineno=_); end

  sig.returns(T.noreturn)
  sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def self.exit(status=_); end

  sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def self.exit!(status); end

  sig.returns(T.noreturn)
  sig(
      arg0: String,
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: T::Array[String],
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def self.fail(arg0=_, arg1=_, arg2=_); end

  sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def self.format(format, *args); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def self.gets(arg0=_, arg1=_); end

  sig.returns(T::Array[Symbol])
  def self.global_variables(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.iterator?(); end

  sig(
      filename: String,
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.load(filename, arg0=_); end

  sig.returns(T::Array[Symbol])
  def self.local_variables(); end

  sig(
      name: String,
      rest: T.any(String, Integer),
      block: String,
  )
  .returns(T.nilable(IO))
  def self.open(name, rest=_, block=_); end

  sig(
      arg0: IO,
      arg1: String,
      arg2: BasicObject,
  )
  .returns(NilClass)
  def self.printf(arg0=_, arg1=_, *arg2); end

  sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def self.proc(&blk); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.putc(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def self.puts(*arg0); end

  sig.returns(T.noreturn)
  sig(
      arg0: String,
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  sig(
      arg0: Exception,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def self.raise(arg0=_, arg1=_, arg2=_); end

  sig.returns(Float)
  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: T::Range[Integer],
  )
  .returns(Integer)
  sig(
      arg0: T::Range[Float],
  )
  .returns(Float)
  def self.rand(arg0=_); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def self.readline(arg0=_, arg1=_); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(T::Array[String])
  def self.readlines(arg0=_, arg1=_); end

  sig(
      name: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.require(name); end

  sig(
      name: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.require_relative(name); end

  sig(
      read: T::Array[IO],
      write: T::Array[IO],
      error: T::Array[IO],
      timeout: Integer,
  )
  .returns(T::Array[String])
  def self.select(read, write=_, error=_, timeout=_); end

  sig(
      duration: Numeric,
  )
  .returns(Integer)
  def self.sleep(duration); end

  sig(
      number: Numeric,
  )
  .returns(Numeric)
  def self.srand(number); end

  sig(
      num: Integer,
      args: BasicObject,
  )
  .returns(T.untyped)
  def self.syscall(num, *args); end

  sig(
      cmd: String,
      file1: String,
      file2: String,
  )
  .returns(T.any(TrueClass, FalseClass, Time))
  def self.test(cmd, file1, file2=_); end

  sig(
      msg: String,
  )
  .returns(NilClass)
  def self.warn(*msg); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def !~(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(other); end

  sig(
      other: BasicObject,
  )
  .returns(NilClass)
  def =~(other); end

  sig.returns(T.self_type)
  def clone(); end

  sig(
      port: IO,
  )
  .returns(NilClass)
  def display(port); end

  sig.returns(T.self_type)
  def dup(); end

  sig(
      method: Symbol,
      args: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  sig(
      method: Symbol,
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  def enum_for(method=_, *args, &blk); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end

  sig(
      mod: Module,
  )
  .returns(NilClass)
  def extend(mod); end

  sig.returns(T.self_type)
  def freeze(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def frozen?(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(String)
  def inspect(); end

  sig(
      arg0: Class,
  )
  .returns(T.any(TrueClass, FalseClass))
  def instance_of?(arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def instance_variable_defined?(arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.untyped)
  def instance_variable_get(arg0); end

  sig(
      arg0: T.any(Symbol, String),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def instance_variable_set(arg0, arg1); end

  sig.returns(T::Array[Symbol])
  def instance_variables(); end

  sig(
      arg0: T.any(Class, Module),
  )
  .returns(T.any(TrueClass, FalseClass))
  def is_a?(arg0); end

  sig(
      arg0: Class,
  )
  .returns(T.any(TrueClass, FalseClass))
  def kind_of?(arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(Method)
  def method(arg0); end

  sig(
      regular: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def methods(regular=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def nil?(); end

  sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def private_methods(all=_); end

  sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def protected_methods(all=_); end

  sig(
      arg0: Symbol,
  )
  .returns(Method)
  def public_method(arg0); end

  sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def public_methods(all=_); end

  sig(
      arg0: T.any(Symbol, String),
      args: BasicObject,
  )
  .returns(T.untyped)
  def public_send(arg0, *args); end

  sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  def remove_instance_variable(arg0); end

  sig(
      arg0: T.any(String, Symbol),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  sig(
      arg0: T.any(String, Symbol),
      arg1: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def send(arg0, *arg1, &blk); end

  sig.returns(Class)
  def singleton_class(); end

  sig(
      arg0: Symbol,
  )
  .returns(Method)
  def singleton_method(arg0); end

  sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def singleton_methods(all=_); end

  sig.returns(T.self_type)
  def taint(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def tainted?(); end

  sig(
      method: Symbol,
      args: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  sig(
      method: Symbol,
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  def to_enum(method=_, *args, &blk); end

  sig.returns(String)
  def to_s(); end

  sig.returns(T.self_type)
  def trust(); end

  sig.returns(T.self_type)
  def untaint(); end

  sig.returns(T.self_type)
  def untrust(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def untrusted?(); end

  type_parameters(:T).sig(
      x: Object,
  )
  .returns(T::Array[T.type_parameter(:T)])
  def Array(x); end

  sig(
      initial: T.any(Integer, Float, Rational, BigDecimal, String),
      digits: Integer,
  )
  .void
  def BigDecimal(initial, digits=0); end

  sig(
      x: T.any(Numeric, String),
      y: T.any(Numeric, String),
  )
  .returns(Complex)
  sig(
      x: String,
  )
  .returns(Complex)
  def Complex(x, y=_); end

  sig(
      x: T.any(Numeric, String),
  )
  .returns(Float)
  def Float(x); end

  type_parameters(:K ,:V).sig(
      x: Object,
  )
  .returns(T::Hash[T.type_parameter(:K), T.type_parameter(:V)])
  def Hash(x); end

  sig(
      arg: T.any(Numeric, String),
      base: Integer,
  )
  .returns(Integer)
  def Integer(arg, base=_); end

  sig(
      x: T.any(Numeric, String),
      y: T.any(Numeric, String),
  )
  .returns(Rational)
  sig(
      x: Object,
  )
  .returns(Rational)
  def Rational(x, y=_); end

  sig(
      x: Object,
  )
  .returns(String)
  def String(x); end

  sig.returns(T.nilable(Symbol))
  def __callee__(); end

  sig.returns(T.nilable(String))
  def __dir__(); end

  sig.returns(T.nilable(Symbol))
  def __method__(); end

  sig(
      arg0: String,
  )
  .returns(String)
  def `(arg0); end

  sig(
      msg: String,
  )
  .returns(T.noreturn)
  def abort(msg=_); end

  sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(Proc)
  def at_exit(&blk); end

  sig(
      _module: T.any(String, Symbol),
      filename: String,
  )
  .returns(NilClass)
  def autoload(_module, filename); end

  sig(
      name: T.any(Symbol, String),
  )
  .returns(T.nilable(String))
  def autoload?(name); end

  sig.returns(Binding)
  def binding(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def block_given?(); end

  sig.returns(T.noreturn)
  sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def exit(status=_); end

  sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def exit!(status); end

  sig.returns(T.noreturn)
  sig(
      arg0: String,
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: T::Array[String],
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def fail(arg0=_, arg1=_, arg2=_); end

  sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def format(format, *args); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def gets(arg0=_, arg1=_); end

  sig.returns(T::Array[Symbol])
  def global_variables(); end

  sig(
      filename: String,
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def load(filename, arg0=_); end

  sig(
      blk: T.proc().returns(T.untyped)
  )
  .returns(NilClass)
  sig.returns(Enumerator[T.untyped])
  def loop(&blk); end

  sig(
      name: String,
      rest: T.any(String, Integer),
      block: String,
  )
  .returns(T.nilable(IO))
  def open(name, rest=_, block=_); end

  sig(
      arg0: IO,
      arg1: String,
      arg2: BasicObject,
  )
  .returns(NilClass)
  def printf(arg0=_, arg1=_, *arg2); end

  sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def proc(&blk); end

  sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def lambda(&blk); end

  sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def self.lambda(&blk); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def putc(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def puts(*arg0); end

  sig.returns(Float)
  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: T::Range[Integer],
  )
  .returns(Integer)
  sig(
      arg0: T::Range[Float],
  )
  .returns(Float)
  def rand(arg0=_); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def readline(arg0=_, arg1=_); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(T::Array[String])
  def readlines(arg0=_, arg1=_); end

  sig(
      path: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def require(path); end

  sig(
      feature: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def require_relative(feature); end

  sig(
      read: T::Array[IO],
      write: T::Array[IO],
      error: T::Array[IO],
      timeout: Integer,
  )
  .returns(T::Array[String])
  def select(read, write=_, error=_, timeout=_); end

  sig(
      duration: Numeric,
  )
  .returns(Integer)
  def sleep(duration); end

  sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def self.sprintf(format, *args); end

  sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def sprintf(format, *args); end

  sig(
      num: Integer,
      args: BasicObject,
  )
  .returns(T.untyped)
  def syscall(num, *args); end

  sig(
      cmd: String,
      file1: String,
      file2: String,
  )
  .returns(T.any(TrueClass, FalseClass, Time))
  def test(cmd, file1, file2=_); end

  sig(
      msg: String,
  )
  .returns(NilClass)
  def warn(*msg); end

  sig.returns(T.noreturn)
  sig(
      arg0: String,
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  sig(
      arg0: Exception,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def raise(arg0=_, arg1=_, arg2=_); end
end
