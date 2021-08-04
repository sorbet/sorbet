# typed: __STDLIB_INTERNAL

# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) is the default
# root of all Ruby objects.
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) inherits from
# [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html) which
# allows creating alternate object hierarchies. Methods on
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) are available to
# all classes unless explicitly overridden.
#
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) mixes in the
# [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) module, making the
# built-in kernel functions globally accessible. Although the instance methods
# of [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) are defined by
# the [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) module, we
# have chosen to document them here for clarity.
#
# When referencing constants in classes inheriting from
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) you do not need to
# use the full namespace. For example, referencing `File` inside `YourClass`
# will find the top-level
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) class.
#
# In the descriptions of Object's methods, the parameter *symbol* refers to a
# symbol, which is either a quoted string or a
# [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) (such as `:name`).
class Object < BasicObject
  include Kernel

  # Returns an integer identifier for `obj`.
  #
  # The same number will be returned on all calls to `object_id` for a given
  # object, and no two active objects will share an id.
  #
  # Note: that some objects of builtin classes are reused for optimization. This
  # is the case for immediate values and frozen string literals.
  #
  # [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html)
  # implements +\_\_id\_\_+,
  # [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) implements
  # `object_id`.
  #
  # Immediate values are not passed by reference but are passed by value: `nil`,
  # `true`, `false`, Fixnums, Symbols, and some Floats.
  #
  # ```ruby
  # Object.new.object_id  == Object.new.object_id  # => false
  # (21 * 2).object_id    == (21 * 2).object_id    # => true
  # "hello".object_id     == "hello".object_id     # => false
  # "hi".freeze.object_id == "hi".freeze.object_id # => true
  # ```
  sig {returns(Integer)}
  def object_id(); end

  # Returns the receiver `obj`.
  #
  # ```ruby
  # obj = Object.new; obj.itself.object_id == o.object_id # => true
  # ```
  sig {returns(T.self_type)}
  def itself(); end

  # Yields self to the block and returns the result of the block.
  #
  # ```ruby
  # 3.next.then {|x| x**x }.to_s             #=> "256"
  # "my string".yield_self {|s| s.upcase }   #=> "MY STRING"
  # ```
  #
  # Good usage for `then` is value piping in method chains:
  #
  # ```ruby
  # require 'open-uri'
  # require 'json'
  #
  # construct_url(arguments).
  #   then {|url| open(url).read }.
  #   then {|response| JSON.parse(response) }
  # ```
  #
  # When called without block, the method returns `Enumerator`, which can be
  # used, for example, for conditional circuit-breaking:
  #
  # ```ruby
  # # meets condition, no-op
  # 1.then.detect(&:odd?)            # => 1
  # # does not meet condition, drop value
  # 2.then.detect(&:odd?)            # => nil
  # ```
  sig do
    type_parameters(:X)
      .params(blk: T.proc.params(arg: T.untyped).returns(T.type_parameter(:X)))
      .returns(T.type_parameter(:X))
  end
  def yield_self(&blk); end

  ### `then` is just an alias of `yield_self`. Separately def'd here for easier IDE integration

  # Yields self to the block and returns the result of the block.
  #
  # ```ruby
  # 3.next.then {|x| x**x }.to_s             #=> "256"
  # "my string".yield_self {|s| s.upcase }   #=> "MY STRING"
  # ```
  #
  # Good usage for `then` is value piping in method chains:
  #
  # ```ruby
  # require 'open-uri'
  # require 'json'
  #
  # construct_url(arguments).
  #   then {|url| open(url).read }.
  #   then {|response| JSON.parse(response) }
  # ```
  #
  # When called without block, the method returns `Enumerator`, which can be
  # used, for example, for conditional circuit-breaking:
  #
  # ```ruby
  # # meets condition, no-op
  # 1.then.detect(&:odd?)            # => 1
  # # does not meet condition, drop value
  # 2.then.detect(&:odd?)            # => nil
  # ```
  sig do
    type_parameters(:X)
      .params(blk: T.proc.params(arg: T.untyped).returns(T.type_parameter(:X)))
      .returns(T.type_parameter(:X))
  end
  def then(&blk); end

  # Kernel methods are available as private instance methods in Object. Full documentation in the Kernel module.

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
  sig {returns(T::Array[String])}
  private def caller(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end

  private_class_method def self.caller(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end

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
  private def caller_locations(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end

  sig do
    params(
        tag: Object,
        blk: T.proc.params(arg0: Object).returns(T.untyped),
    )
    .returns(T.untyped)
  end
  private def catch(tag=Object.new, &blk); end

  sig do
    params(
        arg0: String,
        arg1: T.nilable(Binding),
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  private def eval(arg0, arg1=T.unsafe(nil), filename=T.unsafe(nil), lineno=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  private def iterator?(); end

  sig {returns(T::Array[Symbol])}
  private def local_variables(); end

  sig do
    params(
        number: Numeric,
    )
    .returns(Numeric)
  end
  private def srand(number=T.unsafe(nil)); end

  sig {returns(T.nilable(Integer))}
  sig do
    params(
        blk: T.proc.returns(BasicObject),
    )
    .returns(T.nilable(Integer))
  end
  private def fork(&blk); end

  sig {params(object: T.self_type).returns(T.self_type)}
  private def initialize_copy(object); end

  sig do
    params(
        arg0: Class,
    )
    .returns(T::Boolean)
  end
  private def instance_of?(arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T::Boolean)
  end
  private def instance_variable_defined?(arg0); end

  sig do
    params(
        arg: BasicObject,
    )
    .void
  end
  private def undef(*arg); end

  sig do
    params(
        x: BasicObject,
    )
    .returns(T::Array[T.untyped])
  end
  sig {params(x: NilClass).returns([])}
  private def Array(x); end

  sig do
    params(
        initial: T.any(Integer, Float, Rational, BigDecimal, String),
        digits: Integer,
        exception: T::Boolean
    )
    .returns(BigDecimal)
  end
  private def BigDecimal(initial, digits=0, exception: true); end

  sig do
    params(
        x: T.any(Numeric, String),
        y: T.any(Numeric, String),
        exception: T::Boolean
    )
    .returns(Complex)
  end
  private def Complex(x, y=T.unsafe(nil), exception: false); end

  sig do
    params(
        x: T.any(Numeric, String),
        exception: T::Boolean
    )
    .returns(Float)
  end
  private def Float(x, exception: true); end

  sig do
    type_parameters(:K ,:V).params(
        x: Object,
    )
    .returns(T::Hash[T.type_parameter(:K), T.type_parameter(:V)])
  end
  private def Hash(x); end

  sig do
    params(
        arg: T.any(Numeric, String),
        base: Integer,
        exception: T::Boolean
    )
    .returns(Integer)
  end
  private def Integer(arg, base=T.unsafe(nil), exception: true); end

  sig do
    params(
        x: T.any(Numeric, String, Object),
        y: T.any(Numeric, String),
        exception: T::Boolean
    )
    .returns(Rational)
  end
  private def Rational(x, y=T.unsafe(nil), exception: true); end

  sig do
    params(
        x: Object,
    )
    .returns(String)
  end
  private def String(x); end

  sig {returns(T.nilable(Symbol))}
  private def __callee__(); end

  sig {returns(T.nilable(String))}
  private def __dir__(); end

  sig {returns(T.nilable(Symbol))}
  private def __method__(); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  private def `(arg0); end

  sig do
    params(
        msg: String,
    )
    .returns(T.noreturn)
  end
  private def abort(msg=T.unsafe(nil)); end

  sig do
    params(
        blk: T.proc.returns(BasicObject),
    )
    .returns(Proc)
  end
  private def at_exit(&blk); end

  sig do
    params(
        _module: T.any(String, Symbol),
        filename: String,
    )
    .returns(NilClass)
  end
  private def autoload(_module, filename); end

  sig do
    params(
        name: T.any(Symbol, String),
    )
    .returns(T.nilable(String))
  end
  private def autoload?(name); end

  sig {returns(Binding)}
  private def binding(); end

  sig {returns(T::Boolean)}
  private def block_given?(); end

  sig {returns(T.noreturn)}
  sig do
    params(
        status: T.any(Integer, TrueClass, FalseClass),
    )
    .returns(T.noreturn)
  end
  private def exit(status=T.unsafe(nil)); end

  sig do
    params(
        status: T.any(Integer, TrueClass, FalseClass),
    )
    .returns(T.noreturn)
  end
  private def exit!(status); end

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
  private def fail(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  sig do
    params(
        format: String,
        args: BasicObject,
    )
    .returns(String)
  end
  private def format(format, *args); end

  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(String)
  end
  private def gets(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  sig {returns(T::Array[Symbol])}
  private def global_variables(); end

  sig do
    params(
        filename: String,
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  private def load(filename, arg0=T.unsafe(nil)); end

  sig {params(blk: T.proc.returns(T.untyped)).returns(T.noreturn)}
  sig {returns(T::Enumerator[T.untyped])}
  private def loop(&blk); end

  sig do
    params(
      path: String,
      mode: T.any(Integer, String),
      perm: T.nilable(Integer),
      opt: T.nilable(T::Hash[Symbol, T.untyped]),
    ).returns(T.nilable(IO))
  end
  sig do
    type_parameters(:U).params(
      path: String,
      mode: T.any(Integer, String),
      perm: T.nilable(Integer),
      opt: T.nilable(T::Hash[Symbol, T.untyped]),
      blk: T.proc.params(io: IO).returns(T.type_parameter(:U))
    ).returns(T.type_parameter(:U))
  end
  private def open(path, mode='r', perm=nil, opt=nil, &blk); end

  sig {params(args: Kernel).returns(NilClass)}
  private def print(*args); end

  sig do
    params(
        arg0: IO,
        arg1: String,
        arg2: BasicObject,
    )
    .returns(NilClass)
  end
  private def printf(arg0=T.unsafe(nil), arg1=T.unsafe(nil), *arg2); end

  sig do
    params(
        blk: BasicObject,
    )
    .returns(Proc)
  end
  private def proc(&blk); end

  sig do
    params(
        blk: BasicObject,
    )
    .returns(Proc)
  end
  private def lambda(&blk); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  private def putc(arg0); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  private def puts(*arg0); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  private def p(*arg0); end

  private def pp(obj, out = nil, width = nil); end

  sig {returns(Float)}
  sig do
    params(
        arg0: T.any(Integer, T::Range[Integer]),
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: T.any(T.nilable(Float), T::Range[Float]),
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: T.any(Numeric, T::Range[Numeric]),
    )
    .returns(Numeric)
  end
  private def rand(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(String)
  end
  private def readline(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(T::Array[String])
  end
  private def readlines(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  sig do
    params(
        path: String,
    )
    .returns(T::Boolean)
  end
  private def require(path); end

  sig do
    params(
        feature: T.any(String, Pathname)
    )
    .returns(T::Boolean)
  end
  private def require_relative(feature); end

  sig do
    params(
        read_array: T.nilable(T::Array[IO]),
        write_array: T.nilable(T::Array[IO]),
        error_array: T.nilable(T::Array[IO]),
        timeout: T.any(NilClass, Integer, Float),
    )
    .returns(T.nilable(T::Array[T::Array[IO]]))
  end
  private def select(read_array, write_array=nil, error_array=nil, timeout=nil); end

  sig do
    returns(T.noreturn)
  end
  sig do
    params(
        duration: Numeric,
    )
    .returns(Integer)
  end
  private def sleep(duration); end

  sig do
    params(
        format: String,
        args: BasicObject,
    )
    .returns(String)
  end
  private def sprintf(format, *args); end

  sig do
    params(
        num: Integer,
        args: BasicObject,
    )
    .returns(T.untyped)
  end
  private def syscall(num, *args); end

  sig do
    params(
        cmd: String,
        file1: String,
        file2: String,
    )
    .returns(T.any(TrueClass, FalseClass, Time))
  end
  private def test(cmd, file1, file2=T.unsafe(nil)); end

  sig do
    params(
        tag: Object,
        obj: BasicObject,
    )
    .returns(T.noreturn)
  end
  private def throw(tag, obj=nil); end

  sig do
    params(
        signal: T.any(Integer, String, Symbol),
        command: BasicObject,
    )
    .returns(T.any(String, Proc))
  end
  sig do
    params(
        signal: T.any(Integer, String, Symbol),
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.any(String, Proc))
  end
  private def trap(signal, command=T.unsafe(nil), &blk); end

  sig do
    params(
        msg: String,
    )
    .returns(NilClass)
  end
  private def warn(*msg); end

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
        arg1: T.untyped,
        arg2: T.nilable(T::Array[String]),
    )
    .returns(T.noreturn)
  end
  sig do
    params(
        arg0: Exception,
        arg1: T.untyped,
        arg2: T.nilable(T::Array[String]),
    )
    .returns(T.noreturn)
  end
  private def raise(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  sig { params(args: String).returns(T.noreturn) }
  private def exec(*args); end

  sig do
    params(
      env: T.any(String, [String, String], T::Hash[String, T.nilable(String)]),
      argv0: T.any(String, [String, String]),
      args: String,
      options: T.untyped,
    ).returns(T.nilable(T::Boolean))
  end
  private def system(env, argv0 = T.unsafe(nil), *args, **options); end
end
