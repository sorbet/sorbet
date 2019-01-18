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
        blk: Proc
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

  sig {returns(T.any(TrueClass, FalseClass))}
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
    .returns(T.any(TrueClass, FalseClass))
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
    .returns(T.any(TrueClass, FalseClass))
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
    .returns(T.any(TrueClass, FalseClass))
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

  sig {returns(T.any(TrueClass, FalseClass))}
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
    .returns(T.any(TrueClass, FalseClass))
  end
  def instance_of?(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.any(TrueClass, FalseClass))
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
    .returns(T.any(TrueClass, FalseClass))
  end
  def is_a?(arg0); end

  sig do
    params(
        arg0: Class,
    )
    .returns(T.any(TrueClass, FalseClass))
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
        regular: T.any(TrueClass, FalseClass),
    )
    .returns(T::Array[Symbol])
  end
  def methods(regular=T.unsafe(nil)); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def nil?(); end

  sig do
    params(
        all: T.any(TrueClass, FalseClass),
    )
    .returns(T::Array[Symbol])
  end
  def private_methods(all=T.unsafe(nil)); end

  sig do
    params(
        all: T.any(TrueClass, FalseClass),
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
        all: T.any(TrueClass, FalseClass),
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
        all: T.any(TrueClass, FalseClass),
    )
    .returns(T::Array[Symbol])
  end
  def singleton_methods(all=T.unsafe(nil)); end

  sig {returns(T.self_type)}
  def taint(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def tainted?(); end

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

  sig {returns(T.self_type)}
  def untaint(); end

  sig {returns(T.self_type)}
  def untrust(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def untrusted?(); end

  sig do
    type_parameters(:T).params(
        x: Object,
    )
    .returns(T::Array[T.type_parameter(:T)])
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

  sig {returns(T.any(TrueClass, FalseClass))}
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
        arg0: T.any(TrueClass, FalseClass),
    )
    .returns(T.any(TrueClass, FalseClass))
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
    .returns(T.any(TrueClass, FalseClass))
  end
  def require(path); end

  sig do
    params(
        feature: String,
    )
    .returns(T.any(TrueClass, FalseClass))
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
end
