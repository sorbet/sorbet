# typed: __STDLIB_INTERNAL

# RubyGems adds the
# [`gem`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-gem) method
# to allow activation of specific gem versions and overrides the
# [`require`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-require)
# method on [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) to make
# gems appear as if they live on the `$LOAD_PATH`. See the documentation of
# these methods for further detail.
# The [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) module is
# included by class [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html),
# so its methods are available in every Ruby object.
#
# The [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) instance
# methods are documented in class
# [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) while the module
# methods are documented here. These methods are called without a receiver and
# thus can be called in functional form:
#
# ```ruby
# sprintf "%.1f", 1.234 #=> "1.2"
# ```
#
# fronzen-string-literal: true
module Kernel
  RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)

  # Generates a [`Continuation`](https://ruby-doc.org/3.2.1/Continuation.html)
  # object, which it passes to the associated block. You need to
  # `require 'continuation'` before using this method. Performing a
  # cont.call will cause the
  # [`callcc`](https://ruby-doc.org/3.2.1/Kernel.html#method-i-callcc) to
  # return (as will falling through the end
  # of the block). The value returned by the
  # [`callcc`](https://ruby-doc.org/3.2.1/Kernel.html#method-i-callcc) is the
  # value of the block, or the value passed to cont.call. See class
  # [`Continuation`](https://ruby-doc.org/3.2.1/Continuation.html) for more
  # details. Also see
  # [`Kernel#throw`](https://ruby-doc.org/3.2.1/Kernel.html#method-i-throw)
  # for an alternative mechanism for unwinding a call stack.
  sig do
    type_parameters(:U).params(
      block: T.proc.params(cont: Continuation).returns(T.type_parameter(:U))
    ).returns(T.type_parameter(:U))
  end
  def callcc(&block); end

  ### A note on global functions:
  ###
  ### Ruby tends to define global (e.g. "require", "puts") as
  ### module_function's on Kernel. In practice, this is mostly
  ### superfluous; since Kernel is itself an ancestor of
  ### Kernel.singleton_class, all of Kernel's instance methods will
  ### automatically appear on Kernel directly. For this reason, we omit
  ### module_function and just define these methods as instance methods,
  ### for clarity and simplicity.

  # Returns the current execution stack---an array containing strings in the
  # form `file:line` or `file:line: in `method'`.
  #
  # The optional *start* parameter determines the number of initial stack
  # entries to omit from the top of the stack.
  #
  # A second optional `length` parameter can be used to limit how many entries
  # are returned from the stack.
  #
  # Returns `nil` if *start* is greater than the size of current execution
  # stack.
  #
  # Optionally you can pass a range, which will return an array containing the
  # entries within the specified range.
  #
  # ```ruby
  # def a(skip)
  #   caller(skip)
  # end
  # def b(skip)
  #   a(skip)
  # end
  # def c(skip)
  #   b(skip)
  # end
  # c(0)   #=> ["prog:2:in `a'", "prog:5:in `b'", "prog:8:in `c'", "prog:10:in `<main>'"]
  # c(1)   #=> ["prog:5:in `b'", "prog:8:in `c'", "prog:11:in `<main>'"]
  # c(2)   #=> ["prog:8:in `c'", "prog:12:in `<main>'"]
  # c(3)   #=> ["prog:13:in `<main>'"]
  # c(4)   #=> []
  # c(5)   #=> nil
  # ```
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
  def caller(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end

  # Returns the current execution stack---an array containing strings in the
  # form `file:line` or `file:line: in `method'`.
  #
  # The optional *start* parameter determines the number of initial stack
  # entries to omit from the top of the stack.
  #
  # A second optional `length` parameter can be used to limit how many entries
  # are returned from the stack.
  #
  # Returns `nil` if *start* is greater than the size of current execution
  # stack.
  #
  # Optionally you can pass a range, which will return an array containing the
  # entries within the specified range.
  #
  # ```ruby
  # def a(skip)
  #   caller(skip)
  # end
  # def b(skip)
  #   a(skip)
  # end
  # def c(skip)
  #   b(skip)
  # end
  # c(0)   #=> ["prog:2:in `a'", "prog:5:in `b'", "prog:8:in `c'", "prog:10:in `<main>'"]
  # c(1)   #=> ["prog:5:in `b'", "prog:8:in `c'", "prog:11:in `<main>'"]
  # c(2)   #=> ["prog:8:in `c'", "prog:12:in `<main>'"]
  # c(3)   #=> ["prog:13:in `<main>'"]
  # c(4)   #=> []
  # c(5)   #=> nil
  # ```
  def self.caller(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end

  # Returns the current execution stack---an array containing backtrace location
  # objects.
  #
  # See
  # [`Thread::Backtrace::Location`](https://docs.ruby-lang.org/en/2.7.0/Thread/Backtrace/Location.html)
  # for more information.
  #
  # The optional *start* parameter determines the number of initial stack
  # entries to omit from the top of the stack.
  #
  # A second optional `length` parameter can be used to limit how many entries
  # are returned from the stack.
  #
  # Returns `nil` if *start* is greater than the size of current execution
  # stack.
  #
  # Optionally you can pass a range, which will return an array containing the
  # entries within the specified range.
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

  # `catch` executes its block. If `throw` is not called, the block executes
  # normally, and `catch` returns the value of the last expression evaluated.
  #
  # ```ruby
  # catch(1) { 123 }            # => 123
  # ```
  #
  # If `throw(tag2, val)` is called, Ruby searches up its stack for a `catch`
  # block whose `tag` has the same `object_id` as *tag2*. When found, the block
  # stops executing and returns *val* (or `nil` if no second argument was given
  # to `throw`).
  #
  # ```ruby
  # catch(1) { throw(1, 456) }  # => 456
  # catch(1) { throw(1) }       # => nil
  # ```
  #
  # When `tag` is passed as the first argument, `catch` yields it as the
  # parameter of the block.
  #
  # ```ruby
  # catch(1) {|x| x + 2 }       # => 3
  # ```
  #
  # When no `tag` is given, `catch` yields a new unique object (as from
  # `Object.new`) as the block parameter. This object can then be used as the
  # argument to `throw`, and will match the correct `catch` block.
  #
  # ```ruby
  # catch do |obj_A|
  #   catch do |obj_B|
  #     throw(obj_B, 123)
  #     puts "This puts is not reached"
  #   end
  #
  #   puts "This puts is displayed"
  #   456
  # end
  #
  # # => 456
  #
  # catch do |obj_A|
  #   catch do |obj_B|
  #     throw(obj_A, 123)
  #     puts "This puts is still not reached"
  #   end
  #
  #   puts "Now this puts is also not reached"
  #   456
  # end
  #
  # # => 123
  # ```
  sig do
    params(
        tag: Object,
        blk: T.proc.params(arg0: Object).returns(T.untyped),
    )
    .returns(T.untyped)
  end
  def catch(tag=Object.new, &blk); end

  # Returns the class of *obj*. This method must always be called with an
  # explicit receiver, as
  # [`class`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-class) is
  # also a reserved word in Ruby.
  #
  # ```ruby
  # 1.class      #=> Integer
  # self.class   #=> Object
  # ```
  sig do
    # In a perfect world this should be:
    #
    #   returns(T.class_of(T.self_type))
    #
    # but that doesn't work (yet). Even making it:
    #
    #   returns(Class)
    #
    # is very surprising since users expect their methods to be present.
    # So we settle for untyped.
    returns(T.untyped)
  end
  def class; end

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
        blk: T.untyped
    )
    .returns(Symbol)
  end
  def define_singleton_method(symbol, method=T.unsafe(nil), &blk); end

  # Evaluates the Ruby expression(s) in *string*. If *binding* is given, which
  # must be a [`Binding`](https://docs.ruby-lang.org/en/2.7.0/Binding.html)
  # object, the evaluation is performed in its context. If the optional
  # *filename* and *lineno* parameters are present, they will be used when
  # reporting syntax errors.
  #
  # ```ruby
  # def get_binding(str)
  #   return binding
  # end
  # str = "hello"
  # eval "str + ' Fred'"                      #=> "hello Fred"
  # eval "str + ' Fred'", get_binding("bye")  #=> "bye Fred"
  # ```
  sig do
    params(
        arg0: String,
        arg1: T.nilable(Binding),
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  def eval(arg0, arg1=T.unsafe(nil), filename=T.unsafe(nil), lineno=T.unsafe(nil)); end

  # Deprecated. Use block\_given? instead.
  sig {returns(T::Boolean)}
  def iterator?(); end

  # Returns the names of the current local variables.
  #
  # ```ruby
  # fred = 1
  # for i in 1..10
  #    # ...
  # end
  # local_variables   #=> [:fred, :i]
  # ```
  sig {returns(T::Array[Symbol])}
  def local_variables(); end

  # Seeds the system pseudo-random number generator, with `number`. The previous
  # seed value is returned.
  #
  # If `number` is omitted, seeds the generator using a source of entropy
  # provided by the operating system, if available (/dev/urandom on Unix systems
  # or the RSA cryptographic provider on Windows), which is then combined with
  # the time, the process id, and a sequence number.
  #
  # srand may be used to ensure repeatable sequences of pseudo-random numbers
  # between different runs of the program. By setting the seed to a known value,
  # programs can be made deterministic during testing.
  #
  # ```ruby
  # srand 1234               # => 268519324636777531569100071560086917274
  # [ rand, rand ]           # => [0.1915194503788923, 0.6221087710398319]
  # [ rand(10), rand(1000) ] # => [4, 664]
  # srand 1234               # => 1234
  # [ rand, rand ]           # => [0.1915194503788923, 0.6221087710398319]
  # ```
  sig do
    params(
        number: Numeric,
    )
    .returns(Numeric)
  end
  def srand(number=T.unsafe(nil)); end

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

  # Produces a shallow copy of *obj*---the instance variables of *obj* are
  # copied, but not the objects they reference.
  # [`clone`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-clone)
  # copies the frozen value state of *obj*, unless the `:freeze` keyword
  # argument is given with a false or true value. See also the discussion under
  # [`Object#dup`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-dup).
  #
  # ```ruby
  # class Klass
  #    attr_accessor :str
  # end
  # s1 = Klass.new      #=> #<Klass:0x401b3a38>
  # s1.str = "Hello"    #=> "Hello"
  # s2 = s1.clone       #=> #<Klass:0x401b3998 @str="Hello">
  # s2.str[1,4] = "i"   #=> "i"
  # s1.inspect          #=> "#<Klass:0x401b3a38 @str=\"Hi\">"
  # s2.inspect          #=> "#<Klass:0x401b3998 @str=\"Hi\">"
  # ```
  #
  # This method may have class-specific behavior. If so, that behavior will be
  # documented under the #`initialize_copy` method of the class.
  sig { params(freeze: T.nilable(T::Boolean)).returns(T.self_type) }
  def clone(freeze: nil); end

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
    .returns(T::Enumerator[T.untyped])
  end
  sig do
    params(
        method: Symbol,
        args: BasicObject,
        blk: T.untyped,
    )
    .returns(T::Enumerator[T.untyped])
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
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def extend(*arg0); end

  # Creates a subprocess. If a block is specified, that block is run in the
  # subprocess, and the subprocess terminates with a status of zero. Otherwise,
  # the `fork` call returns twice, once in the parent, returning the process ID
  # of the child, and once in the child, returning *nil*. The child process can
  # exit using
  # [`Kernel.exit!`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-exit-21)
  # to avoid running any `at_exit` functions. The parent process should use
  # [`Process.wait`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-wait)
  # to collect the termination statuses of its children or use
  # [`Process.detach`](https://docs.ruby-lang.org/en/2.7.0/Process.html#method-c-detach)
  # to register disinterest in their status; otherwise, the operating system may
  # accumulate zombie processes.
  #
  # The thread calling fork is the only thread in the created child process.
  # fork doesn't copy other threads.
  #
  # If fork is not usable, Process.respond\_to?(:fork) returns false.
  #
  # Note that fork(2) is not available on some platforms like Windows and NetBSD
  # 4. Therefore you should use spawn() instead of fork().
  sig {returns(T.nilable(Integer))}
  sig do
    params(
        blk: T.proc.returns(BasicObject),
    )
    .returns(Integer)
  end
  def fork(&blk); end

  sig {returns(T.self_type)}
  def freeze(); end

  # Returns the freeze status of *obj*.
  #
  # ```ruby
  # a = [ "a", "b", "c" ]
  # a.freeze    #=> ["a", "b", "c"]
  # a.frozen?   #=> true
  # ```
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
        arg0: T::Class[T.anything],
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
        arg0: T.any(Module),
    )
    .returns(T::Boolean)
  end
  def is_a?(arg0); end

  sig do
    params(
        arg0: Module,
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
        blk: T.untyped
    )
    .returns(T.untyped)
  end
  def public_send(arg0, *args, &blk); end

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
        include_all: T.untyped,
    )
    .returns(T::Boolean)
  end
  def respond_to?(arg0,include_all=false); end

  sig do
    params(
        arg0: T.any(String, Symbol),
        arg1: T.anything,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        arg0: T.any(String, Symbol),
        arg1: T.anything,
        blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def send(arg0, *arg1, &blk); end

  sig {returns(T::Class[T.anything])}
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

  # Yields self to the block, and then returns self. The primary purpose of this
  # method is to "tap into" a method chain, in order to perform operations on
  # intermediate results within the chain.
  #
  # ```ruby
  # (1..10)                  .tap {|x| puts "original: #{x}" }
  #   .to_a                  .tap {|x| puts "array:    #{x}" }
  #   .select {|x| x.even? } .tap {|x| puts "evens:    #{x}" }
  #   .map {|x| x*x }        .tap {|x| puts "squares:  #{x}" }
  # ```
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
    .returns(T::Enumerator[T.untyped])
  end
  sig do
    params(
        method: Symbol,
        args: BasicObject,
        blk: T.untyped,
    )
    .returns(T::Enumerator[T.untyped])
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

  # Returns `arg` as an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  #
  # First tries to call `to_ary` on `arg`, then `to_a`. If `arg` does not
  # respond to `to_ary` or `to_a`, returns an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of length 1
  # containing `arg`.
  #
  # If `to_ary` or `to_a` returns something other than an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html), raises a
  # [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html).
  #
  # ```ruby
  # Array(["a", "b"])  #=> ["a", "b"]
  # Array(1..5)        #=> [1, 2, 3, 4, 5]
  # Array(key: :value) #=> [[:key, :value]]
  # Array(nil)         #=> []
  # Array(1)           #=> [1]
  # ```
  sig { params(x: NilClass).returns(T::Array[T.untyped]) }
  sig do
    type_parameters(:Elem)
      .params(
        x: T.any(T::Enumerable[T.type_parameter(:Elem)], T.type_parameter(:Elem), T.nilable(T.type_parameter(:Elem)))
      )
      .returns(T::Array[T.type_parameter(:Elem)])
  end
  def Array(x); end

  # ```
  # Returns the \BigDecimal converted from +value+
  # with a precision of +ndigits+ decimal digits.
  #
  # When +ndigits+ is less than the number of significant digits
  # in the value, the result is rounded to that number of digits,
  # according to the current rounding mode; see BigDecimal.mode.
  # ```
  #
  # Returns `value` converted to a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html),
  # depending on the type of `value`:
  #
  # *   [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html),
  #     [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html),
  #     [`Rational`](https://docs.ruby-lang.org/en/2.7.0/Rational.html),
  #     [`Complex`](https://docs.ruby-lang.org/en/2.7.0/Complex.html), or
  #     BigDecimal: converted directly:
  #
  # ```ruby
  # # Integer, Complex, or BigDecimal value does not require ndigits; ignored if given.
  # BigDecimal(2)                     # => 0.2e1
  # BigDecimal(Complex(2, 0))         # => 0.2e1
  # BigDecimal(BigDecimal(2))         # => 0.2e1
  # # Float or Rational value requires ndigits.
  # BigDecimal(2.0, 0)                # => 0.2e1
  # BigDecimal(Rational(2, 1), 0)     # => 0.2e1
  # ```
  #
  # *   String: converted by parsing if it contains an integer or floating-point
  #     literal; leading and trailing whitespace is ignored:
  #
  # ```ruby
  # # String does not require ndigits; ignored if given.
  # BigDecimal('2')     # => 0.2e1
  # BigDecimal('2.0')   # => 0.2e1
  # BigDecimal('0.2e1') # => 0.2e1
  # BigDecimal(' 2.0 ') # => 0.2e1
  # ```
  #
  # *   Other type that responds to method `:to_str`: first converted to a
  #     string, then converted to a
  #     [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html), as
  #     above.
  #
  # *   Other type:
  #
  #     *   Raises an exception if keyword argument `exception` is `true`.
  #     *   Returns `nil` if keyword argument `exception` is `true`.
  #
  #
  #
  # Raises an exception if `value` evaluates to a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) and `digits` is
  # larger than Float::DIG + 1.
  sig do
    params(
        initial: T.any(Integer, Float, Rational, BigDecimal, String),
        digits: Integer,
        exception: T::Boolean
    )
    .returns(BigDecimal)
  end
  def BigDecimal(initial, digits=0, exception: true); end
  ### As of 2.6, all these numeric conversion methods *can* return `nil` iff given the kwarg `exception: false`, which
  ### means the non-nilable return value is technically incorrect, but it seemed like the least worst option given that
  ### 1) usage of the exception kwarg is uncommon, and 2) Sorbet doesn't allow us to overload sigs if methods have kwargs
  ### see this PR for discussion/context: https://github.com/sorbet/sorbet/pull/1144

  # Returns x+i\*y;
  #
  # ```ruby
  # Complex(1, 2)    #=> (1+2i)
  # Complex('1+2i')  #=> (1+2i)
  # Complex(nil)     #=> TypeError
  # Complex(1, nil)  #=> TypeError
  #
  # Complex(1, nil, exception: false)  #=> nil
  # Complex('1+2', exception: false)   #=> nil
  # ```
  #
  # Syntax of string form:
  #
  # ```
  # string form = extra spaces , complex , extra spaces ;
  # complex = real part | [ sign ] , imaginary part
  #         | real part , sign , imaginary part
  #         | rational , "@" , rational ;
  # real part = rational ;
  # imaginary part = imaginary unit | unsigned rational , imaginary unit ;
  # rational = [ sign ] , unsigned rational ;
  # unsigned rational = numerator | numerator , "/" , denominator ;
  # numerator = integer part | fractional part | integer part , fractional part ;
  # denominator = digits ;
  # integer part = digits ;
  # fractional part = "." , digits , [ ( "e" | "E" ) , [ sign ] , digits ] ;
  # imaginary unit = "i" | "I" | "j" | "J" ;
  # sign = "-" | "+" ;
  # digits = digit , { digit | "_" , digit };
  # digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
  # extra spaces = ? \s* ? ;
  # ```
  #
  # See
  # [`String#to_c`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-to_c).
  sig do
    params(
        x: T.any(Numeric, String),
        y: T.any(Numeric, String),
        exception: T::Boolean
    )
    .returns(Complex)
  end
  def Complex(x, y=T.unsafe(nil), exception: false); end
  ### As of 2.6, all these numeric conversion methods *can* return `nil` iff given the kwarg `exception: false`, which
  ### means the non-nilable return value is technically incorrect, but it seemed like the least worst option given that
  ### 1) usage of the exception kwarg is uncommon, and 2) Sorbet doesn't allow us to overload sigs if methods have kwargs
  ### see this PR for discussion/context: https://github.com/sorbet/sorbet/pull/1144

  # Returns *arg* converted to a float.
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) types are
  # converted directly, and with exception to
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) and `nil` the
  # rest are converted using *arg*`.to_f`. Converting a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) with invalid
  # characters will result in a
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html).
  # Converting `nil` generates a
  # [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html).
  # Exceptions can be suppressed by passing `exception: false`.
  #
  # ```ruby
  # Float(1)                 #=> 1.0
  # Float("123.456")         #=> 123.456
  # Float("123.0_badstring") #=> ArgumentError: invalid value for Float(): "123.0_badstring"
  # Float(nil)               #=> TypeError: can't convert nil into Float
  # Float("123.0_badstring", exception: false)  #=> nil
  # ```
  sig do
    params(
        x: T.any(Numeric, String),
        exception: T::Boolean
    )
    .returns(Float)
  end
  def Float(x, exception: true); end
  ### As of 2.6, all these numeric conversion methods *can* return `nil` iff given the kwarg `exception: false`, which
  ### means the non-nilable return value is technically incorrect, but it seemed like the least worst option given that
  ### 1) usage of the exception kwarg is uncommon, and 2) Sorbet doesn't allow us to overload sigs if methods have kwargs
  ### see this PR for discussion/context: https://github.com/sorbet/sorbet/pull/1144

  # Converts *arg* to a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  # by calling *arg*`.to_hash`. Returns an empty
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) when *arg* is `nil`
  # or `[]`.
  #
  # ```ruby
  # Hash([])          #=> {}
  # Hash(nil)         #=> {}
  # Hash(key: :value) #=> {:key => :value}
  # Hash([1, 2, 3])   #=> TypeError
  # ```
  sig do
    type_parameters(:K ,:V).params(
        x: Object,
    )
    .returns(T::Hash[T.type_parameter(:K), T.type_parameter(:V)])
  end
  def Hash(x); end

  # Converts *arg* to an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) types are
  # converted directly (with floating point numbers being truncated). *base* (0,
  # or between 2 and 36) is a base for integer string representation. If *arg*
  # is a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), when
  # *base* is omitted or equals zero, radix indicators (`0`, `0b`, and `0x`) are
  # honored. In any case, strings should consist only of one or more digits,
  # except for that a sign, one underscore between two digits, and
  # leading/trailing spaces are optional. This behavior is different from that
  # of
  # [`String#to_i`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-to_i).
  # Non string values will be converted by first trying `to_int`, then `to_i`.
  #
  # Passing `nil` raises a
  # [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html), while
  # passing a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) that
  # does not conform with numeric representation raises an
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html).
  # This behavior can be altered by passing `exception: false`, in this case a
  # not convertible value will return `nil`.
  #
  # ```ruby
  # Integer(123.999)    #=> 123
  # Integer("0x1a")     #=> 26
  # Integer(Time.new)   #=> 1204973019
  # Integer("0930", 10) #=> 930
  # Integer("111", 2)   #=> 7
  # Integer(" +1_0 ")   #=> 10
  # Integer(nil)        #=> TypeError: can't convert nil into Integer
  # Integer("x")        #=> ArgumentError: invalid value for Integer(): "x"
  #
  # Integer("x", exception: false)        #=> nil
  # ```
  sig do
    params(
        arg: T.any(Numeric, String),
        base: Integer,
        exception: T::Boolean
    )
    .returns(Integer)
  end
  def Integer(arg, base=T.unsafe(nil), exception: true); end
  ### As of 2.6, all these numeric conversion methods *can* return `nil` iff given the kwarg `exception: false`, which
  ### means the non-nilable return value is technically incorrect, but it seemed like the least worst option given that
  ### 1) usage of the exception kwarg is uncommon, and 2) Sorbet doesn't allow us to overload sigs if methods have kwargs
  ### see this PR for discussion/context: https://github.com/sorbet/sorbet/pull/1144

  # Returns `x/y` or `arg` as a
  # [`Rational`](https://docs.ruby-lang.org/en/2.7.0/Rational.html).
  #
  # ```ruby
  # Rational(2, 3)   #=> (2/3)
  # Rational(5)      #=> (5/1)
  # Rational(0.5)    #=> (1/2)
  # Rational(0.3)    #=> (5404319552844595/18014398509481984)
  #
  # Rational("2/3")  #=> (2/3)
  # Rational("0.3")  #=> (3/10)
  #
  # Rational("10 cents")  #=> ArgumentError
  # Rational(nil)         #=> TypeError
  # Rational(1, nil)      #=> TypeError
  #
  # Rational("10 cents", exception: false)  #=> nil
  # ```
  #
  # Syntax of the string form:
  #
  # ```
  # string form = extra spaces , rational , extra spaces ;
  # rational = [ sign ] , unsigned rational ;
  # unsigned rational = numerator | numerator , "/" , denominator ;
  # numerator = integer part | fractional part | integer part , fractional part ;
  # denominator = digits ;
  # integer part = digits ;
  # fractional part = "." , digits , [ ( "e" | "E" ) , [ sign ] , digits ] ;
  # sign = "-" | "+" ;
  # digits = digit , { digit | "_" , digit } ;
  # digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
  # extra spaces = ? \s* ? ;
  # ```
  #
  # See also
  # [`String#to_r`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-to_r).
  sig do
    params(
        x: T.any(Numeric, String, Object),
        y: T.any(Numeric, String),
        exception: T::Boolean
    )
    .returns(Rational)
  end
  def Rational(x, y=T.unsafe(nil), exception: true); end
  ### As of 2.6, all these numeric conversion methods *can* return `nil` iff given the kwarg `exception: false`, which
  ### means the non-nilable return value is technically incorrect, but it seemed like the least worst option given that
  ### 1) usage of the exception kwarg is uncommon, and 2) Sorbet doesn't allow us to overload sigs if methods have kwargs
  ### see this PR for discussion/context: https://github.com/sorbet/sorbet/pull/1144

  # Returns *arg* as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  # First tries to call its `to_str` method, then its `to_s` method.
  #
  # ```ruby
  # String(self)        #=> "main"
  # String(self.class)  #=> "Object"
  # String(123456)      #=> "123456"
  # ```
  sig do
    params(
        x: Object,
    )
    .returns(String)
  end
  def String(x); end

  # Returns the called name of the current method as a
  # [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html). If called
  # outside of a method, it returns `nil`.
  sig {returns(T.nilable(Symbol))}
  def __callee__(); end

  # Returns the canonicalized absolute path of the directory of the file from
  # which this method is called. It means symlinks in the path is resolved. If
  # `__FILE__` is `nil`, it returns `nil`. The return value equals to
  # `File.dirname(File.realpath(__FILE__))`.
  sig {returns(T.nilable(String))}
  def __dir__(); end

  # Returns the name at the definition of the current method as a
  # [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html). If called
  # outside of a method, it returns `nil`.
  sig {returns(T.nilable(Symbol))}
  def __method__(); end

  # Returns the standard output of running *cmd* in a subshell. The built-in
  # syntax `%x{...}` uses this method. Sets `$?` to the process status.
  #
  # ```ruby
  # `date`                   #=> "Wed Apr  9 08:56:30 CDT 2003\n"
  # `ls testdir`.split[1]    #=> "main.rb"
  # `echo oops && exit 99`   #=> "oops\n"
  # $?.exitstatus            #=> 99
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def `(arg0); end

  # Terminate execution immediately, effectively by calling
  # `Kernel.exit(false)`. If *msg* is given, it is written to STDERR prior to
  # terminating.
  sig do
    params(
        msg: String,
    )
    .returns(T.noreturn)
  end
  def abort(msg=T.unsafe(nil)); end

  # Converts *block* to a `Proc` object (and therefore binds it at the point of
  # call) and registers it for execution when the program exits. If multiple
  # handlers are registered, they are executed in reverse order of registration.
  #
  # ```ruby
  # def do_at_exit(str1)
  #   at_exit { print str1 }
  # end
  # at_exit { puts "cruel world" }
  # do_at_exit("goodbye ")
  # exit
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # goodbye cruel world
  # ```
  sig do
    params(
        blk: T.proc.returns(BasicObject),
    )
    .returns(Proc)
  end
  def at_exit(&blk); end

  # Registers *filename* to be loaded (using Kernel::require) the first time
  # that *module* (which may be a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or a symbol) is
  # accessed.
  #
  # ```ruby
  # autoload(:MyModule, "/usr/local/lib/modules/my_module.rb")
  # ```
  sig do
    params(
        _module: T.any(String, Symbol),
        filename: String,
    )
    .returns(NilClass)
  end
  def autoload(_module, filename); end

  # Returns *filename* to be loaded if *name* is registered as `autoload`.
  #
  # ```ruby
  # autoload(:B, "b")
  # autoload?(:B)            #=> "b"
  # ```
  sig do
    params(
        name: T.any(Symbol, String),
    )
    .returns(T.nilable(String))
  end
  def autoload?(name); end

  # Returns a `Binding` object, describing the variable and method bindings at
  # the point of call. This object can be used when calling `eval` to execute
  # the evaluated command in this environment. See also the description of class
  # `Binding`.
  #
  # ```ruby
  # def get_binding(param)
  #   binding
  # end
  # b = get_binding("hello")
  # eval("param", b)   #=> "hello"
  # ```
  sig {returns(Binding)}
  def binding(); end

  # Returns `true` if `yield` would execute a block in the current context. The
  # `iterator?` form is mildly deprecated.
  #
  # ```ruby
  # def try
  #   if block_given?
  #     yield
  #   else
  #     "no block"
  #   end
  # end
  # try                  #=> "no block"
  # try { "hello" }      #=> "hello"
  # try do "hello" end   #=> "hello"
  # ```
  sig {returns(T::Boolean)}
  def block_given?(); end

  # Initiates the termination of the Ruby script by raising the
  # [`SystemExit`](https://docs.ruby-lang.org/en/2.7.0/SystemExit.html)
  # exception. This exception may be caught. The optional parameter is used to
  # return a status code to the invoking environment. `true` and `FALSE` of
  # *status* means success and failure respectively. The interpretation of other
  # integer values are system dependent.
  #
  # ```ruby
  # begin
  #   exit
  #   puts "never get here"
  # rescue SystemExit
  #   puts "rescued a SystemExit exception"
  # end
  # puts "after begin block"
  # ```
  #
  # *produces:*
  #
  # ```
  # rescued a SystemExit exception
  # after begin block
  # ```
  #
  # Just prior to termination, Ruby executes any `at_exit` functions (see
  # Kernel::at\_exit) and runs any object finalizers (see
  # [`ObjectSpace::define_finalizer`](https://docs.ruby-lang.org/en/2.7.0/ObjectSpace.html#method-c-define_finalizer)).
  #
  # ```ruby
  # at_exit { puts "at_exit function" }
  # ObjectSpace.define_finalizer("string",  proc { puts "in finalizer" })
  # exit
  # ```
  #
  # *produces:*
  #
  # ```
  # at_exit function
  # in finalizer
  # ```
  sig {returns(T.noreturn)}
  sig do
    params(
        status: T.any(Integer, TrueClass, FalseClass),
    )
    .returns(T.noreturn)
  end
  def exit(status=T.unsafe(nil)); end

  # Exits the process immediately. No exit handlers are run. *status* is
  # returned to the underlying system as the exit status.
  #
  # ```ruby
  # Process.exit!(true)
  # ```
  sig do
    params(
        status: T.any(Integer, TrueClass, FalseClass),
    )
    .returns(T.noreturn)
  end
  def exit!(status=T.unsafe(nil)); end

  # With no arguments, raises the exception in `$!` or raises a
  # [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html) if
  # `$!` is `nil`. With a single `String` argument, raises a `RuntimeError` with
  # the string as a message. Otherwise, the first parameter should be an
  # `Exception` class (or another object that returns an `Exception` object when
  # sent an `exception` message). The optional second parameter sets the message
  # associated with the exception (accessible via
  # [`Exception#message`](https://docs.ruby-lang.org/en/2.7.0/Exception.html#method-i-message)),
  # and the third parameter is an array of callback information (accessible via
  # [`Exception#backtrace`](https://docs.ruby-lang.org/en/2.7.0/Exception.html#method-i-backtrace)).
  # The `cause` of the generated exception (accessible via
  # [`Exception#cause`](https://docs.ruby-lang.org/en/2.7.0/Exception.html#method-i-cause))
  # is automatically set to the "current" exception (`$!`), if any. An
  # alternative value, either an `Exception` object or `nil`, can be specified
  # via the `:cause` argument.
  #
  # Exceptions are caught by the `rescue` clause of `begin...end` blocks.
  #
  # ```ruby
  # raise "Failed to create socket"
  # raise ArgumentError, "No parameters", caller
  # ```
  #
  #
  # Alias for:
  # [`raise`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-raise)
  sig {returns(T.noreturn)}
  sig do
    params(
        arg0: String,
    )
    .returns(T.noreturn)
  end
  sig do
    params(
        arg0: T::Class[T.anything],
        arg1: T.any(String, T::Array[String]),
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
  def fail(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  # Returns the string resulting from applying *format\_string* to any
  # additional arguments. Within the format string, any characters other than
  # format sequences are copied to the result.
  #
  # The syntax of a format sequence is as follows.
  #
  # ```
  # %[flags][width][.precision]type
  # ```
  #
  # A format sequence consists of a percent sign, followed by optional flags,
  # width, and precision indicators, then terminated with a field type
  # character. The field type controls how the corresponding `sprintf` argument
  # is to be interpreted, while the flags modify that interpretation.
  #
  # The field type characters are:
  #
  # ```
  # Field |  Integer Format
  # ------+--------------------------------------------------------------
  #   b   | Convert argument as a binary number.
  #       | Negative numbers will be displayed as a two's complement
  #       | prefixed with `..1'.
  #   B   | Equivalent to `b', but uses an uppercase 0B for prefix
  #       | in the alternative format by #.
  #   d   | Convert argument as a decimal number.
  #   i   | Identical to `d'.
  #   o   | Convert argument as an octal number.
  #       | Negative numbers will be displayed as a two's complement
  #       | prefixed with `..7'.
  #   u   | Identical to `d'.
  #   x   | Convert argument as a hexadecimal number.
  #       | Negative numbers will be displayed as a two's complement
  #       | prefixed with `..f' (representing an infinite string of
  #       | leading 'ff's).
  #   X   | Equivalent to `x', but uses uppercase letters.
  #
  # Field |  Float Format
  # ------+--------------------------------------------------------------
  #   e   | Convert floating point argument into exponential notation
  #       | with one digit before the decimal point as [-]d.dddddde[+-]dd.
  #       | The precision specifies the number of digits after the decimal
  #       | point (defaulting to six).
  #   E   | Equivalent to `e', but uses an uppercase E to indicate
  #       | the exponent.
  #   f   | Convert floating point argument as [-]ddd.dddddd,
  #       | where the precision specifies the number of digits after
  #       | the decimal point.
  #   g   | Convert a floating point number using exponential form
  #       | if the exponent is less than -4 or greater than or
  #       | equal to the precision, or in dd.dddd form otherwise.
  #       | The precision specifies the number of significant digits.
  #   G   | Equivalent to `g', but use an uppercase `E' in exponent form.
  #   a   | Convert floating point argument as [-]0xh.hhhhp[+-]dd,
  #       | which is consisted from optional sign, "0x", fraction part
  #       | as hexadecimal, "p", and exponential part as decimal.
  #   A   | Equivalent to `a', but use uppercase `X' and `P'.
  #
  # Field |  Other Format
  # ------+--------------------------------------------------------------
  #   c   | Argument is the numeric code for a single character or
  #       | a single character string itself.
  #   p   | The valuing of argument.inspect.
  #   s   | Argument is a string to be substituted.  If the format
  #       | sequence contains a precision, at most that many characters
  #       | will be copied.
  #   %   | A percent sign itself will be displayed.  No argument taken.
  # ```
  #
  # The flags modifies the behavior of the formats. The flag characters are:
  #
  # ```
  # Flag     | Applies to    | Meaning
  # ---------+---------------+-----------------------------------------
  # space    | bBdiouxX      | Leave a space at the start of
  #          | aAeEfgG       | non-negative numbers.
  #          | (numeric fmt) | For `o', `x', `X', `b' and `B', use
  #          |               | a minus sign with absolute value for
  #          |               | negative values.
  # ---------+---------------+-----------------------------------------
  # (digit)$ | all           | Specifies the absolute argument number
  #          |               | for this field.  Absolute and relative
  #          |               | argument numbers cannot be mixed in a
  #          |               | sprintf string.
  # ---------+---------------+-----------------------------------------
  #  #       | bBoxX         | Use an alternative format.
  #          | aAeEfgG       | For the conversions `o', increase the precision
  #          |               | until the first digit will be `0' if
  #          |               | it is not formatted as complements.
  #          |               | For the conversions `x', `X', `b' and `B'
  #          |               | on non-zero, prefix the result with ``0x'',
  #          |               | ``0X'', ``0b'' and ``0B'', respectively.
  #          |               | For `a', `A', `e', `E', `f', `g', and 'G',
  #          |               | force a decimal point to be added,
  #          |               | even if no digits follow.
  #          |               | For `g' and 'G', do not remove trailing zeros.
  # ---------+---------------+-----------------------------------------
  # +        | bBdiouxX      | Add a leading plus sign to non-negative
  #          | aAeEfgG       | numbers.
  #          | (numeric fmt) | For `o', `x', `X', `b' and `B', use
  #          |               | a minus sign with absolute value for
  #          |               | negative values.
  # ---------+---------------+-----------------------------------------
  # -        | all           | Left-justify the result of this conversion.
  # ---------+---------------+-----------------------------------------
  # 0 (zero) | bBdiouxX      | Pad with zeros, not spaces.
  #          | aAeEfgG       | For `o', `x', `X', `b' and `B', radix-1
  #          | (numeric fmt) | is used for negative numbers formatted as
  #          |               | complements.
  # ---------+---------------+-----------------------------------------
  # *        | all           | Use the next argument as the field width.
  #          |               | If negative, left-justify the result. If the
  #          |               | asterisk is followed by a number and a dollar
  #          |               | sign, use the indicated argument as the width.
  # ```
  #
  # Examples of flags:
  #
  # ```ruby
  # # `+' and space flag specifies the sign of non-negative numbers.
  # sprintf("%d", 123)  #=> "123"
  # sprintf("%+d", 123) #=> "+123"
  # sprintf("% d", 123) #=> " 123"
  #
  # # `#' flag for `o' increases number of digits to show `0'.
  # # `+' and space flag changes format of negative numbers.
  # sprintf("%o", 123)   #=> "173"
  # sprintf("%#o", 123)  #=> "0173"
  # sprintf("%+o", -123) #=> "-173"
  # sprintf("%o", -123)  #=> "..7605"
  # sprintf("%#o", -123) #=> "..7605"
  #
  # # `#' flag for `x' add a prefix `0x' for non-zero numbers.
  # # `+' and space flag disables complements for negative numbers.
  # sprintf("%x", 123)   #=> "7b"
  # sprintf("%#x", 123)  #=> "0x7b"
  # sprintf("%+x", -123) #=> "-7b"
  # sprintf("%x", -123)  #=> "..f85"
  # sprintf("%#x", -123) #=> "0x..f85"
  # sprintf("%#x", 0)    #=> "0"
  #
  # # `#' for `X' uses the prefix `0X'.
  # sprintf("%X", 123)  #=> "7B"
  # sprintf("%#X", 123) #=> "0X7B"
  #
  # # `#' flag for `b' add a prefix `0b' for non-zero numbers.
  # # `+' and space flag disables complements for negative numbers.
  # sprintf("%b", 123)   #=> "1111011"
  # sprintf("%#b", 123)  #=> "0b1111011"
  # sprintf("%+b", -123) #=> "-1111011"
  # sprintf("%b", -123)  #=> "..10000101"
  # sprintf("%#b", -123) #=> "0b..10000101"
  # sprintf("%#b", 0)    #=> "0"
  #
  # # `#' for `B' uses the prefix `0B'.
  # sprintf("%B", 123)  #=> "1111011"
  # sprintf("%#B", 123) #=> "0B1111011"
  #
  # # `#' for `e' forces to show the decimal point.
  # sprintf("%.0e", 1)  #=> "1e+00"
  # sprintf("%#.0e", 1) #=> "1.e+00"
  #
  # # `#' for `f' forces to show the decimal point.
  # sprintf("%.0f", 1234)  #=> "1234"
  # sprintf("%#.0f", 1234) #=> "1234."
  #
  # # `#' for `g' forces to show the decimal point.
  # # It also disables stripping lowest zeros.
  # sprintf("%g", 123.4)   #=> "123.4"
  # sprintf("%#g", 123.4)  #=> "123.400"
  # sprintf("%g", 123456)  #=> "123456"
  # sprintf("%#g", 123456) #=> "123456."
  # ```
  #
  # The field width is an optional integer, followed optionally by a period and
  # a precision. The width specifies the minimum number of characters that will
  # be written to the result for this field.
  #
  # Examples of width:
  #
  # ```ruby
  # # padding is done by spaces,       width=20
  # # 0 or radix-1.             <------------------>
  # sprintf("%20d", 123)   #=> "                 123"
  # sprintf("%+20d", 123)  #=> "                +123"
  # sprintf("%020d", 123)  #=> "00000000000000000123"
  # sprintf("%+020d", 123) #=> "+0000000000000000123"
  # sprintf("% 020d", 123) #=> " 0000000000000000123"
  # sprintf("%-20d", 123)  #=> "123                 "
  # sprintf("%-+20d", 123) #=> "+123                "
  # sprintf("%- 20d", 123) #=> " 123                "
  # sprintf("%020x", -123) #=> "..ffffffffffffffff85"
  # ```
  #
  # For numeric fields, the precision controls the number of decimal places
  # displayed. For string fields, the precision determines the maximum number of
  # characters to be copied from the string. (Thus, the format sequence
  # `%10.10s` will always contribute exactly ten characters to the result.)
  #
  # Examples of precisions:
  #
  # ```ruby
  # # precision for `d', 'o', 'x' and 'b' is
  # # minimum number of digits               <------>
  # sprintf("%20.8d", 123)  #=> "            00000123"
  # sprintf("%20.8o", 123)  #=> "            00000173"
  # sprintf("%20.8x", 123)  #=> "            0000007b"
  # sprintf("%20.8b", 123)  #=> "            01111011"
  # sprintf("%20.8d", -123) #=> "           -00000123"
  # sprintf("%20.8o", -123) #=> "            ..777605"
  # sprintf("%20.8x", -123) #=> "            ..ffff85"
  # sprintf("%20.8b", -11)  #=> "            ..110101"
  #
  # # "0x" and "0b" for `#x' and `#b' is not counted for
  # # precision but "0" for `#o' is counted.  <------>
  # sprintf("%#20.8d", 123)  #=> "            00000123"
  # sprintf("%#20.8o", 123)  #=> "            00000173"
  # sprintf("%#20.8x", 123)  #=> "          0x0000007b"
  # sprintf("%#20.8b", 123)  #=> "          0b01111011"
  # sprintf("%#20.8d", -123) #=> "           -00000123"
  # sprintf("%#20.8o", -123) #=> "            ..777605"
  # sprintf("%#20.8x", -123) #=> "          0x..ffff85"
  # sprintf("%#20.8b", -11)  #=> "          0b..110101"
  #
  # # precision for `e' is number of
  # # digits after the decimal point           <------>
  # sprintf("%20.8e", 1234.56789) #=> "      1.23456789e+03"
  #
  # # precision for `f' is number of
  # # digits after the decimal point               <------>
  # sprintf("%20.8f", 1234.56789) #=> "       1234.56789000"
  #
  # # precision for `g' is number of
  # # significant digits                          <------->
  # sprintf("%20.8g", 1234.56789) #=> "           1234.5679"
  #
  # #                                         <------->
  # sprintf("%20.8g", 123456789)  #=> "       1.2345679e+08"
  #
  # # precision for `s' is
  # # maximum number of characters                    <------>
  # sprintf("%20.8s", "string test") #=> "            string t"
  # ```
  #
  # Examples:
  #
  # ```ruby
  # sprintf("%d %04x", 123, 123)               #=> "123 007b"
  # sprintf("%08b '%4s'", 123, 123)            #=> "01111011 ' 123'"
  # sprintf("%1$*2$s %2$d %1$s", "hello", 8)   #=> "   hello 8 hello"
  # sprintf("%1$*2$s %2$d", "hello", -8)       #=> "hello    -8"
  # sprintf("%+g:% g:%-g", 1.23, 1.23, 1.23)   #=> "+1.23: 1.23:1.23"
  # sprintf("%u", -123)                        #=> "-123"
  # ```
  #
  # For more complex formatting, Ruby supports a reference by name. %<name>s
  # style uses format style, but %{name} style doesn't.
  #
  # Examples:
  #
  # ```ruby
  # sprintf("%<foo>d : %<bar>f", { :foo => 1, :bar => 2 })
  #   #=> 1 : 2.000000
  # sprintf("%{foo}f", { :foo => 1 })
  #   # => "1f"
  # ```
  #
  #
  # Alias for:
  # [`sprintf`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-sprintf)
  sig do
    params(
        format: String,
        args: BasicObject,
    )
    .returns(String)
  end
  def format(format, *args); end

  # Returns (and assigns to `$_`) the next line from the list of files in `ARGV`
  # (or `$*`), or from standard input if no files are present on the command
  # line. Returns `nil` at end of file. The optional argument specifies the
  # record separator. The separator is included with the contents of each
  # record. A separator of `nil` reads the entire contents, and a zero-length
  # separator reads the input one paragraph at a time, where paragraphs are
  # divided by two consecutive newlines. If the first argument is an integer, or
  # optional second argument is given, the returning string would not be longer
  # than the given value in bytes. If multiple filenames are present in `ARGV`,
  # `gets(nil)` will read the contents one file at a time.
  #
  # ```ruby
  # ARGV << "testfile"
  # print while gets
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # This is line one
  # This is line two
  # This is line three
  # And so on...
  # ```
  #
  # The style of programming using `$_` as an implicit parameter is gradually
  # losing favor in the Ruby community.
  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(String)
  end
  def gets(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  # Returns an array of the names of global variables. This includes special
  # regexp global variables such as `$~` and `$+`, but does not include the
  # numbered regexp global variables (`$1`, `$2`, etc.).
  #
  # ```ruby
  # global_variables.grep /std/   #=> [:$stdin, :$stdout, :$stderr]
  # ```
  sig {returns(T::Array[Symbol])}
  def global_variables(); end

  # Loads and executes the Ruby program in the file *filename*.
  #
  # If the filename is an absolute path (e.g. starts with '/'), the file will be
  # loaded directly using the absolute path.
  #
  # If the filename is an explicit relative path (e.g. starts with './' or
  # '../'), the file will be loaded using the relative path from the current
  # directory.
  #
  # Otherwise, the file will be searched for in the library directories listed
  # in `$LOAD_PATH` (`$:`). If the file is found in a directory, it will attempt
  # to load the file relative to that directory. If the file is not found in any
  # of the directories in `$LOAD_PATH`, the file will be loaded using the
  # relative path from the current directory.
  #
  # If the file doesn't exist when there is an attempt to load it, a
  # [`LoadError`](https://docs.ruby-lang.org/en/2.7.0/LoadError.html) will be
  # raised.
  #
  # If the optional *wrap* parameter is `true`, the loaded script will be
  # executed under an anonymous module, protecting the calling program's global
  # namespace. If the optional *wrap* parameter is a module, the loaded script
  # will be executed under the given module. In no circumstance will any local
  # variables in the loaded file be propagated to the loading environment.
  sig do
    params(
        filename: String,
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def load(filename, arg0=T.unsafe(nil)); end

  # Repeatedly executes the block.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # loop do
  #   print "Input: "
  #   line = gets
  #   break if !line or line =~ /^qQ/
  #   # ...
  # end
  # ```
  #
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.7.0/StopIteration.html)
  # raised in the block breaks the loop. In this case, loop returns the "result"
  # value stored in the exception.
  #
  # ```ruby
  # enum = Enumerator.new { |y|
  #   y << "one"
  #   y << "two"
  #   :ok
  # }
  #
  # result = loop {
  #   puts enum.next
  # } #=> :ok
  # ```
  sig {params(blk: T.proc.returns(T.untyped)).returns(T.noreturn)}
  sig {returns(T::Enumerator[T.untyped])}
  def loop(&blk); end

  # Creates an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object
  # connected to the given stream, file, or subprocess.
  #
  # If `path` does not start with a pipe character (`|`), treat it as the name
  # of a file to open using the specified mode (defaulting to "r").
  #
  # The `mode` is either a string or an integer. If it is an integer, it must be
  # bitwise-or of open(2) flags, such as File::RDWR or File::EXCL. If it is a
  # string, it is either "fmode", "fmode:ext\_enc", or
  # "fmode:ext\_enc:int\_enc".
  #
  # See the documentation of
  # [`IO.new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new) for
  # full documentation of the `mode` string directives.
  #
  # If a file is being created, its initial permissions may be set using the
  # `perm` parameter. See
  # [`File.new`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-new) and
  # the open(2) and chmod(2) man pages for a description of permissions.
  #
  # If a block is specified, it will be invoked with the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object as a parameter,
  # and the [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) will be
  # automatically closed when the block terminates. The call returns the value
  # of the block.
  #
  # If `path` starts with a pipe character (`"|"`), a subprocess is created,
  # connected to the caller by a pair of pipes. The returned
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object may be used to
  # write to the standard input and read from the standard output of this
  # subprocess.
  #
  # If the command following the pipe is a single minus sign (`"|-"`), Ruby
  # forks, and this subprocess is connected to the parent. If the command is not
  # `"-"`, the subprocess runs the command. Note that the command may be
  # processed by shell if it contains shell metacharacters.
  #
  # When the subprocess is Ruby (opened via `"|-"`), the `open` call returns
  # `nil`. If a block is associated with the open call, that block will run
  # twice --- once in the parent and once in the child.
  #
  # The block parameter will be an
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object in the parent and
  # `nil` in the child. The parent's `IO` object will be connected to the
  # child's $stdin and $stdout. The subprocess will be terminated at the end of
  # the block.
  #
  # ### Examples
  #
  # Reading from "testfile":
  #
  # ```ruby
  # open("testfile") do |f|
  #   print f.gets
  # end
  # ```
  #
  # Produces:
  #
  # ```ruby
  # This is line one
  # ```
  #
  # Open a subprocess and read its output:
  #
  # ```ruby
  # cmd = open("|date")
  # print cmd.gets
  # cmd.close
  # ```
  #
  # Produces:
  #
  # ```
  # Wed Apr  9 08:56:31 CDT 2003
  # ```
  #
  # Open a subprocess running the same Ruby program:
  #
  # ```ruby
  # f = open("|-", "w+")
  # if f.nil?
  #   puts "in Child"
  #   exit
  # else
  #   puts "Got: #{f.gets}"
  # end
  # ```
  #
  # Produces:
  #
  # ```
  # Got: in Child
  # ```
  #
  # Open a subprocess using a block to receive the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object:
  #
  # ```ruby
  # open "|-" do |f|
  #   if f then
  #     # parent process
  #     puts "Got: #{f.gets}"
  #   else
  #     # child process
  #     puts "in Child"
  #   end
  # end
  # ```
  #
  # Produces:
  #
  # ```
  # Got: in Child
  # ```
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
  def open(path, mode='r', perm=nil, opt=nil, &blk); end

  # Prints each object in turn to `$stdout`. If the output field separator
  # (`$,`) is not `nil`, its contents will appear between each field. If the
  # output record separator (`$\`) is not `nil`, it will be appended to the
  # output. If no arguments are given, prints `$_`. Objects that aren't strings
  # will be converted by calling their `to_s` method.
  #
  # ```ruby
  # print "cat", [1,2,3], 99, "\n"
  # $, = ", "
  # $\ = "\n"
  # print "cat", [1,2,3], 99
  # ```
  #
  # *produces:*
  #
  # ```
  # cat12399
  # cat, 1, 2, 3, 99
  # ```
  sig {params(args: Kernel).returns(NilClass)}
  def print(*args); end

  # Equivalent to:
  #
  # ```
  # io.write(sprintf(string, obj, ...))
  # ```
  #
  # or
  #
  # ```
  # $stdout.write(sprintf(string, obj, ...))
  # ```
  sig do
    params(
        arg0: IO,
        arg1: String,
        arg2: BasicObject,
    )
    .returns(NilClass)
  end
  def printf(arg0=T.unsafe(nil), arg1=T.unsafe(nil), *arg2); end

  # Equivalent to
  # [`Proc.new`](https://docs.ruby-lang.org/en/2.7.0/Proc.html#method-c-new).
  sig do
    params(
        blk: T.untyped,
    )
    .returns(Proc)
  end
  def proc(&blk); end

  # Equivalent to
  # [`Proc.new`](https://docs.ruby-lang.org/en/2.7.0/Proc.html#method-c-new),
  # except the resulting [`Proc`](https://docs.ruby-lang.org/en/2.7.0/Proc.html)
  # objects check the number of parameters passed when called.
  sig do
    params(
        blk: T.untyped,
    )
    .returns(Proc)
  end
  def lambda(&blk); end

  # Equivalent to:
  #
  # ```ruby
  # $stdout.putc(int)
  # ```
  #
  # Refer to the documentation for
  # [`IO#putc`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-putc) for
  # important information regarding multi-byte characters.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def putc(arg0); end

  # Equivalent to
  #
  # ```
  # $stdout.puts(obj, ...)
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def puts(*arg0); end

  ### TODO: this type is not correct. Observe this irb session:
  ###
  ### >> p
  ### => nil
  ### >> p 1
  ### 1
  ### => 1
  ### >> p 1, 2
  ### 1
  ### 2
  ### => [1, 2]

  # For each object, directly writes *obj*.`inspect` followed by a newline to
  # the program's standard output.
  #
  # ```ruby
  # S = Struct.new(:name, :state)
  # s = S['dave', 'TX']
  # p s
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # #<S name="dave", state="TX">
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def p(*arg0); end

  # prints arguments in pretty form.
  #
  # pp returns argument(s).
  #
  # Also aliased as:
  # [`pp`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-pp)
  def pp(obj, out = nil, width = nil); end

  # If called without an argument, or if `max.to_i.abs == 0`, rand returns a
  # pseudo-random floating point number between 0.0 and 1.0, including 0.0 and
  # excluding 1.0.
  #
  # ```ruby
  # rand        #=> 0.2725926052826416
  # ```
  #
  # When `max.abs` is greater than or equal to 1, `rand` returns a pseudo-random
  # integer greater than or equal to 0 and less than `max.to_i.abs`.
  #
  # ```ruby
  # rand(100)   #=> 12
  # ```
  #
  # When `max` is a [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html),
  # `rand` returns a random number where range.member?(number) == true.
  #
  # Negative or floating point values for `max` are allowed, but may give
  # surprising results.
  #
  # ```ruby
  # rand(-100) # => 87
  # rand(-0.5) # => 0.8130921818028143
  # rand(1.9)  # equivalent to rand(1), which is always 0
  # ```
  #
  # [`Kernel.srand`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-srand)
  # may be used to ensure that sequences of random numbers are reproducible
  # between different runs of a program.
  #
  # See also
  # [`Random.rand`](https://docs.ruby-lang.org/en/2.7.0/Random.html#method-c-rand).
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
  def rand(arg0=T.unsafe(nil)); end

  # Equivalent to Kernel::gets, except `readline` raises `EOFError` at end of
  # file.
  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(String)
  end
  def readline(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  # Returns an array containing the lines returned by calling `Kernel.gets(sep)`
  # until the end of file.
  sig do
    params(
        arg0: String,
        arg1: Integer,
    )
    .returns(T::Array[String])
  end
  def readlines(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  # Loads the given `name`, returning `true` if successful and `false` if the
  # feature is already loaded.
  #
  # If the filename does not resolve to an absolute path, it will be searched
  # for in the directories listed in `$LOAD_PATH` (`$:`).
  #
  # If the filename has the extension ".rb", it is loaded as a source file; if
  # the extension is ".so", ".o", or ".dll", or the default shared library
  # extension on the current platform, Ruby loads the shared library as a Ruby
  # extension. Otherwise, Ruby tries adding ".rb", ".so", and so on to the name
  # until found. If the file named cannot be found, a
  # [`LoadError`](https://docs.ruby-lang.org/en/2.6.0/LoadError.html) will be
  # raised.
  #
  # For Ruby extensions the filename given may use any shared library extension.
  # For example, on Linux the socket extension is "socket.so" and `require
  # 'socket.dll'` will load the socket extension.
  #
  # The absolute path of the loaded file is added to `$LOADED_FEATURES` (`$"`).
  # A file will not be loaded again if its path already appears in `$"`. For
  # example, `require 'a'; require './a'` will not load `a.rb` again.
  #
  # ```ruby
  # require "my-library.rb"
  # require "db-driver"
  # ```
  #
  # Any constants or globals within the loaded source file will be available in
  # the calling program's global namespace. However, local variables will not be
  # propagated to the loading environment.
  #
  # When RubyGems is required,
  # [`#require`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-require)
  # is replaced with our own which is capable of loading gems on demand.
  #
  # When you call `require 'x'`, this is what happens:
  # *   If the file can be loaded from the existing Ruby loadpath, it is.
  # *   Otherwise, installed gems are searched for a file that matches. If it's
  #     found in gem 'y', that gem is activated (added to the loadpath).
  #
  #
  # The normal `require` functionality of returning false if that file has
  # already been loaded is preserved.
  #
  # Also aliased as:
  # [`gem_original_require`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-gem_original_require)
  sig do
    params(
        path: String,
    )
    .returns(T::Boolean)
  end
  def require(path); end

  # Ruby tries to load the library named *string* relative to the requiring
  # file's path. If the file's path cannot be determined a
  # [`LoadError`](https://docs.ruby-lang.org/en/2.7.0/LoadError.html) is raised.
  # If a file is loaded `true` is returned and false otherwise.
  sig do
    params(
        feature: T.any(String, Pathname)
    )
    .returns(T::Boolean)
  end
  def require_relative(feature); end

  # Calls select(2) system call. It monitors given arrays of
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects, waits until one
  # or more of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects are
  # ready for reading, are ready for writing, and have pending exceptions
  # respectively, and returns an array that contains arrays of those
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects. It will return
  # `nil` if optional *timeout* value is given and no
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object is ready in
  # *timeout* seconds.
  #
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # peeks the buffer of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # objects for testing readability. If the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) buffer is not empty,
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # immediately notifies readability. This "peek" only happens for
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects. It does not
  # happen for IO-like objects such as
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html).
  #
  # The best way to use
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # is invoking it after nonblocking methods such as read\_nonblock,
  # write\_nonblock, etc. The methods raise an exception which is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # or
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html).
  # The modules notify how the caller should wait with
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
  # If
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # is raised, the caller should wait for reading. If
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # is raised, the caller should wait for writing.
  #
  # So, blocking read (readpartial) can be emulated using read\_nonblock and
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # as follows:
  #
  # ```ruby
  # begin
  #   result = io_like.read_nonblock(maxlen)
  # rescue IO::WaitReadable
  #   IO.select([io_like])
  #   retry
  # rescue IO::WaitWritable
  #   IO.select(nil, [io_like])
  #   retry
  # end
  # ```
  #
  # Especially, the combination of nonblocking methods and
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # is preferred for [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) like
  # objects such as
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html).
  # It has to\_io method to return underlying
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # calls to\_io to obtain the file descriptor to wait.
  #
  # This means that readability notified by
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # doesn't mean readability from
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html)
  # object.
  #
  # The most likely situation is that
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html)
  # buffers some data.
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # doesn't see the buffer. So
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # can block when
  # [`OpenSSL::SSL::SSLSocket#readpartial`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-readpartial)
  # doesn't block.
  #
  # However, several more complicated situations exist.
  #
  # SSL is a protocol which is sequence of records. The record consists of
  # multiple bytes. So, the remote side of SSL sends a partial record,
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # notifies readability but
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html)
  # cannot decrypt a byte and
  # [`OpenSSL::SSL::SSLSocket#readpartial`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-readpartial)
  # will block.
  #
  # Also, the remote side can request SSL renegotiation which forces the local
  # SSL engine to write some data. This means
  # [`OpenSSL::SSL::SSLSocket#readpartial`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-readpartial)
  # may invoke write system call and it can block. In such a situation,
  # [`OpenSSL::SSL::SSLSocket#read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # raises
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # instead of blocking. So, the caller should wait for ready for writability as
  # above example.
  #
  # The combination of nonblocking methods and
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # is also useful for streams such as tty, pipe socket socket when multiple
  # processes read from a stream.
  #
  # Finally, Linux kernel developers don't guarantee that readability of
  # select(2) means readability of following read(2) even for a single process.
  # See select(2) manual on GNU/Linux system.
  #
  # Invoking
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # before
  # [`IO#readpartial`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readpartial)
  # works well as usual. However it is not the best way to use
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
  #
  # The writability notified by select(2) doesn't show how many bytes are
  # writable.
  # [`IO#write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write)
  # method blocks until given whole string is written. So, `IO#write(two or more
  # bytes)` can block after writability is notified by
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
  # [`IO#write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
  # is required to avoid the blocking.
  #
  # Blocking write (write) can be emulated using write\_nonblock and
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # as follows:
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # should also be rescued for SSL renegotiation in
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html).
  #
  # ```ruby
  # while 0 < string.bytesize
  #   begin
  #     written = io_like.write_nonblock(string)
  #   rescue IO::WaitReadable
  #     IO.select([io_like])
  #     retry
  #   rescue IO::WaitWritable
  #     IO.select(nil, [io_like])
  #     retry
  #   end
  #   string = string.byteslice(written..-1)
  # end
  # ```
  #
  # ### Parameters
  # read\_array
  # :   an array of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects
  #     that wait until ready for read
  # write\_array
  # :   an array of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects
  #     that wait until ready for write
  # error\_array
  # :   an array of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects
  #     that wait for exceptions
  # timeout
  # :   a numeric value in second
  #
  #
  # ### Example
  #
  # ```ruby
  # rp, wp = IO.pipe
  # mesg = "ping "
  # 100.times {
  #   # IO.select follows IO#read.  Not the best way to use IO.select.
  #   rs, ws, = IO.select([rp], [wp])
  #   if r = rs[0]
  #     ret = r.read(5)
  #     print ret
  #     case ret
  #     when /ping/
  #       mesg = "pong\n"
  #     when /pong/
  #       mesg = "ping "
  #     end
  #   end
  #   if w = ws[0]
  #     w.write(mesg)
  #   end
  # }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # ping pong
  # ping pong
  # ping pong
  # (snipped)
  # ping
  # ```
  sig do
    params(
        read_array: T.nilable(T::Array[IO]),
        write_array: T.nilable(T::Array[IO]),
        error_array: T.nilable(T::Array[IO]),
        timeout: T.any(NilClass, Integer, Float),
    )
    .returns(T.nilable(T::Array[T::Array[IO]]))
  end
  def select(read_array, write_array=nil, error_array=nil, timeout=nil); end

  # Establishes _proc_ as the handler for tracing, or disables
  # tracing if the parameter is +nil+.
  #
  # *Note:* this method is obsolete, please use TracePoint instead.
  #
  # _proc_ takes up to six parameters:
  #
  # *   an event name
  # *   a filename
  # *   a line number
  # *   an object id
  # *   a binding
  # *   the name of a class
  #
  # _proc_ is invoked whenever an event occurs.
  #
  # Events are:
  #
  # +c-call+:: call a C-language routine
  # +c-return+:: return from a C-language routine
  # +call+:: call a Ruby method
  # +class+:: start a class or module definition
  # +end+:: finish a class or module definition
  # +line+:: execute code on a new line
  # +raise+:: raise an exception
  # +return+:: return from a Ruby method
  #
  # Tracing is disabled within the context of _proc_.
  #
  #     class Test
  #     def test
  #       a = 1
  #       b = 2
  #     end
  #     end
  #
  #     set_trace_func proc { |event, file, line, id, binding, classname|
  #        printf "%8s %s:%-2d %10s %8s\n", event, file, line, id, classname
  #     }
  #     t = Test.new
  #     t.test
  #
  #       line prog.rb:11               false
  #     c-call prog.rb:11        new    Class
  #     c-call prog.rb:11 initialize   Object
  #   c-return prog.rb:11 initialize   Object
  #   c-return prog.rb:11        new    Class
  #       line prog.rb:12               false
  #       call prog.rb:2        test     Test
  #       line prog.rb:3        test     Test
  #       line prog.rb:4        test     Test
  #     return prog.rb:4        test     Test
  sig do
    params(
      arg0: T.nilable(
        T.proc.params(
          event: String,
          file: String,
          line: Integer,
          id: T.nilable(Symbol),
          binding: T.nilable(Binding),
          classname: Object,
        ).returns(T.untyped)
      )
    ).void
  end
  sig { params(arg0: NilClass).returns(NilClass) }
  def set_trace_func(arg0); end

  # Suspends the current thread for *duration* seconds (which may be any number,
  # including a `Float` with fractional seconds). Returns the actual number of
  # seconds slept (rounded), which may be less than that asked for if another
  # thread calls
  # [`Thread#run`](https://docs.ruby-lang.org/en/2.7.0/Thread.html#method-i-run).
  # Called without an argument, sleep() will sleep forever.
  #
  # ```ruby
  # Time.new    #=> 2008-03-08 19:56:19 +0900
  # sleep 1.2   #=> 1
  # Time.new    #=> 2008-03-08 19:56:20 +0900
  # sleep 1.9   #=> 2
  # Time.new    #=> 2008-03-08 19:56:22 +0900
  # ```
  sig do
    returns(T.noreturn)
  end
  sig do
    params(
        duration: Numeric,
    )
    .returns(Integer)
  end
  def sleep(duration); end

  # Returns the string resulting from applying *format\_string* to any
  # additional arguments. Within the format string, any characters other than
  # format sequences are copied to the result.
  #
  # The syntax of a format sequence is as follows.
  #
  # ```
  # %[flags][width][.precision]type
  # ```
  #
  # A format sequence consists of a percent sign, followed by optional flags,
  # width, and precision indicators, then terminated with a field type
  # character. The field type controls how the corresponding `sprintf` argument
  # is to be interpreted, while the flags modify that interpretation.
  #
  # The field type characters are:
  #
  # ```
  # Field |  Integer Format
  # ------+--------------------------------------------------------------
  #   b   | Convert argument as a binary number.
  #       | Negative numbers will be displayed as a two's complement
  #       | prefixed with `..1'.
  #   B   | Equivalent to `b', but uses an uppercase 0B for prefix
  #       | in the alternative format by #.
  #   d   | Convert argument as a decimal number.
  #   i   | Identical to `d'.
  #   o   | Convert argument as an octal number.
  #       | Negative numbers will be displayed as a two's complement
  #       | prefixed with `..7'.
  #   u   | Identical to `d'.
  #   x   | Convert argument as a hexadecimal number.
  #       | Negative numbers will be displayed as a two's complement
  #       | prefixed with `..f' (representing an infinite string of
  #       | leading 'ff's).
  #   X   | Equivalent to `x', but uses uppercase letters.
  #
  # Field |  Float Format
  # ------+--------------------------------------------------------------
  #   e   | Convert floating point argument into exponential notation
  #       | with one digit before the decimal point as [-]d.dddddde[+-]dd.
  #       | The precision specifies the number of digits after the decimal
  #       | point (defaulting to six).
  #   E   | Equivalent to `e', but uses an uppercase E to indicate
  #       | the exponent.
  #   f   | Convert floating point argument as [-]ddd.dddddd,
  #       | where the precision specifies the number of digits after
  #       | the decimal point.
  #   g   | Convert a floating point number using exponential form
  #       | if the exponent is less than -4 or greater than or
  #       | equal to the precision, or in dd.dddd form otherwise.
  #       | The precision specifies the number of significant digits.
  #   G   | Equivalent to `g', but use an uppercase `E' in exponent form.
  #   a   | Convert floating point argument as [-]0xh.hhhhp[+-]dd,
  #       | which is consisted from optional sign, "0x", fraction part
  #       | as hexadecimal, "p", and exponential part as decimal.
  #   A   | Equivalent to `a', but use uppercase `X' and `P'.
  #
  # Field |  Other Format
  # ------+--------------------------------------------------------------
  #   c   | Argument is the numeric code for a single character or
  #       | a single character string itself.
  #   p   | The valuing of argument.inspect.
  #   s   | Argument is a string to be substituted.  If the format
  #       | sequence contains a precision, at most that many characters
  #       | will be copied.
  #   %   | A percent sign itself will be displayed.  No argument taken.
  # ```
  #
  # The flags modifies the behavior of the formats. The flag characters are:
  #
  # ```
  # Flag     | Applies to    | Meaning
  # ---------+---------------+-----------------------------------------
  # space    | bBdiouxX      | Leave a space at the start of
  #          | aAeEfgG       | non-negative numbers.
  #          | (numeric fmt) | For `o', `x', `X', `b' and `B', use
  #          |               | a minus sign with absolute value for
  #          |               | negative values.
  # ---------+---------------+-----------------------------------------
  # (digit)$ | all           | Specifies the absolute argument number
  #          |               | for this field.  Absolute and relative
  #          |               | argument numbers cannot be mixed in a
  #          |               | sprintf string.
  # ---------+---------------+-----------------------------------------
  #  #       | bBoxX         | Use an alternative format.
  #          | aAeEfgG       | For the conversions `o', increase the precision
  #          |               | until the first digit will be `0' if
  #          |               | it is not formatted as complements.
  #          |               | For the conversions `x', `X', `b' and `B'
  #          |               | on non-zero, prefix the result with ``0x'',
  #          |               | ``0X'', ``0b'' and ``0B'', respectively.
  #          |               | For `a', `A', `e', `E', `f', `g', and 'G',
  #          |               | force a decimal point to be added,
  #          |               | even if no digits follow.
  #          |               | For `g' and 'G', do not remove trailing zeros.
  # ---------+---------------+-----------------------------------------
  # +        | bBdiouxX      | Add a leading plus sign to non-negative
  #          | aAeEfgG       | numbers.
  #          | (numeric fmt) | For `o', `x', `X', `b' and `B', use
  #          |               | a minus sign with absolute value for
  #          |               | negative values.
  # ---------+---------------+-----------------------------------------
  # -        | all           | Left-justify the result of this conversion.
  # ---------+---------------+-----------------------------------------
  # 0 (zero) | bBdiouxX      | Pad with zeros, not spaces.
  #          | aAeEfgG       | For `o', `x', `X', `b' and `B', radix-1
  #          | (numeric fmt) | is used for negative numbers formatted as
  #          |               | complements.
  # ---------+---------------+-----------------------------------------
  # *        | all           | Use the next argument as the field width.
  #          |               | If negative, left-justify the result. If the
  #          |               | asterisk is followed by a number and a dollar
  #          |               | sign, use the indicated argument as the width.
  # ```
  #
  # Examples of flags:
  #
  # ```ruby
  # # `+' and space flag specifies the sign of non-negative numbers.
  # sprintf("%d", 123)  #=> "123"
  # sprintf("%+d", 123) #=> "+123"
  # sprintf("% d", 123) #=> " 123"
  #
  # # `#' flag for `o' increases number of digits to show `0'.
  # # `+' and space flag changes format of negative numbers.
  # sprintf("%o", 123)   #=> "173"
  # sprintf("%#o", 123)  #=> "0173"
  # sprintf("%+o", -123) #=> "-173"
  # sprintf("%o", -123)  #=> "..7605"
  # sprintf("%#o", -123) #=> "..7605"
  #
  # # `#' flag for `x' add a prefix `0x' for non-zero numbers.
  # # `+' and space flag disables complements for negative numbers.
  # sprintf("%x", 123)   #=> "7b"
  # sprintf("%#x", 123)  #=> "0x7b"
  # sprintf("%+x", -123) #=> "-7b"
  # sprintf("%x", -123)  #=> "..f85"
  # sprintf("%#x", -123) #=> "0x..f85"
  # sprintf("%#x", 0)    #=> "0"
  #
  # # `#' for `X' uses the prefix `0X'.
  # sprintf("%X", 123)  #=> "7B"
  # sprintf("%#X", 123) #=> "0X7B"
  #
  # # `#' flag for `b' add a prefix `0b' for non-zero numbers.
  # # `+' and space flag disables complements for negative numbers.
  # sprintf("%b", 123)   #=> "1111011"
  # sprintf("%#b", 123)  #=> "0b1111011"
  # sprintf("%+b", -123) #=> "-1111011"
  # sprintf("%b", -123)  #=> "..10000101"
  # sprintf("%#b", -123) #=> "0b..10000101"
  # sprintf("%#b", 0)    #=> "0"
  #
  # # `#' for `B' uses the prefix `0B'.
  # sprintf("%B", 123)  #=> "1111011"
  # sprintf("%#B", 123) #=> "0B1111011"
  #
  # # `#' for `e' forces to show the decimal point.
  # sprintf("%.0e", 1)  #=> "1e+00"
  # sprintf("%#.0e", 1) #=> "1.e+00"
  #
  # # `#' for `f' forces to show the decimal point.
  # sprintf("%.0f", 1234)  #=> "1234"
  # sprintf("%#.0f", 1234) #=> "1234."
  #
  # # `#' for `g' forces to show the decimal point.
  # # It also disables stripping lowest zeros.
  # sprintf("%g", 123.4)   #=> "123.4"
  # sprintf("%#g", 123.4)  #=> "123.400"
  # sprintf("%g", 123456)  #=> "123456"
  # sprintf("%#g", 123456) #=> "123456."
  # ```
  #
  # The field width is an optional integer, followed optionally by a period and
  # a precision. The width specifies the minimum number of characters that will
  # be written to the result for this field.
  #
  # Examples of width:
  #
  # ```ruby
  # # padding is done by spaces,       width=20
  # # 0 or radix-1.             <------------------>
  # sprintf("%20d", 123)   #=> "                 123"
  # sprintf("%+20d", 123)  #=> "                +123"
  # sprintf("%020d", 123)  #=> "00000000000000000123"
  # sprintf("%+020d", 123) #=> "+0000000000000000123"
  # sprintf("% 020d", 123) #=> " 0000000000000000123"
  # sprintf("%-20d", 123)  #=> "123                 "
  # sprintf("%-+20d", 123) #=> "+123                "
  # sprintf("%- 20d", 123) #=> " 123                "
  # sprintf("%020x", -123) #=> "..ffffffffffffffff85"
  # ```
  #
  # For numeric fields, the precision controls the number of decimal places
  # displayed. For string fields, the precision determines the maximum number of
  # characters to be copied from the string. (Thus, the format sequence
  # `%10.10s` will always contribute exactly ten characters to the result.)
  #
  # Examples of precisions:
  #
  # ```ruby
  # # precision for `d', 'o', 'x' and 'b' is
  # # minimum number of digits               <------>
  # sprintf("%20.8d", 123)  #=> "            00000123"
  # sprintf("%20.8o", 123)  #=> "            00000173"
  # sprintf("%20.8x", 123)  #=> "            0000007b"
  # sprintf("%20.8b", 123)  #=> "            01111011"
  # sprintf("%20.8d", -123) #=> "           -00000123"
  # sprintf("%20.8o", -123) #=> "            ..777605"
  # sprintf("%20.8x", -123) #=> "            ..ffff85"
  # sprintf("%20.8b", -11)  #=> "            ..110101"
  #
  # # "0x" and "0b" for `#x' and `#b' is not counted for
  # # precision but "0" for `#o' is counted.  <------>
  # sprintf("%#20.8d", 123)  #=> "            00000123"
  # sprintf("%#20.8o", 123)  #=> "            00000173"
  # sprintf("%#20.8x", 123)  #=> "          0x0000007b"
  # sprintf("%#20.8b", 123)  #=> "          0b01111011"
  # sprintf("%#20.8d", -123) #=> "           -00000123"
  # sprintf("%#20.8o", -123) #=> "            ..777605"
  # sprintf("%#20.8x", -123) #=> "          0x..ffff85"
  # sprintf("%#20.8b", -11)  #=> "          0b..110101"
  #
  # # precision for `e' is number of
  # # digits after the decimal point           <------>
  # sprintf("%20.8e", 1234.56789) #=> "      1.23456789e+03"
  #
  # # precision for `f' is number of
  # # digits after the decimal point               <------>
  # sprintf("%20.8f", 1234.56789) #=> "       1234.56789000"
  #
  # # precision for `g' is number of
  # # significant digits                          <------->
  # sprintf("%20.8g", 1234.56789) #=> "           1234.5679"
  #
  # #                                         <------->
  # sprintf("%20.8g", 123456789)  #=> "       1.2345679e+08"
  #
  # # precision for `s' is
  # # maximum number of characters                    <------>
  # sprintf("%20.8s", "string test") #=> "            string t"
  # ```
  #
  # Examples:
  #
  # ```ruby
  # sprintf("%d %04x", 123, 123)               #=> "123 007b"
  # sprintf("%08b '%4s'", 123, 123)            #=> "01111011 ' 123'"
  # sprintf("%1$*2$s %2$d %1$s", "hello", 8)   #=> "   hello 8 hello"
  # sprintf("%1$*2$s %2$d", "hello", -8)       #=> "hello    -8"
  # sprintf("%+g:% g:%-g", 1.23, 1.23, 1.23)   #=> "+1.23: 1.23:1.23"
  # sprintf("%u", -123)                        #=> "-123"
  # ```
  #
  # For more complex formatting, Ruby supports a reference by name. %<name>s
  # style uses format style, but %{name} style doesn't.
  #
  # Examples:
  #
  # ```ruby
  # sprintf("%<foo>d : %<bar>f", { :foo => 1, :bar => 2 })
  #   #=> 1 : 2.000000
  # sprintf("%{foo}f", { :foo => 1 })
  #   # => "1f"
  # ```
  #
  #
  # Also aliased as:
  # [`format`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-format)
  sig do
    params(
        format: String,
        args: BasicObject,
    )
    .returns(String)
  end
  def sprintf(format, *args); end

  # Calls the operating system function identified by *num* and returns the
  # result of the function or raises
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # if it failed.
  #
  # Arguments for the function can follow *num*. They must be either `String`
  # objects or `Integer` objects. A `String` object is passed as a pointer to
  # the byte sequence. An `Integer` object is passed as an integer whose bit
  # size is the same as a pointer. Up to nine parameters may be passed.
  #
  # The function identified by *num* is system dependent. On some Unix systems,
  # the numbers may be obtained from a header file called `syscall.h`.
  #
  # ```ruby
  # syscall 4, 1, "hello\n", 6   # '4' is write(2) on our box
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # hello
  # ```
  #
  # Calling `syscall` on a platform which does not have any way to an arbitrary
  # system function just fails with
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html).
  #
  # **Note:** `syscall` is essentially unsafe and unportable. Feel free to shoot
  # your foot. The DL
  # ([`Fiddle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html)) library is
  # preferred for safer and a bit more portable programming.
  sig do
    params(
        num: Integer,
        args: BasicObject,
    )
    .returns(T.untyped)
  end
  def syscall(num, *args); end

  # Uses the character `cmd` to perform various tests on `file1` (first table
  # below) or on `file1` and `file2` (second table).
  #
  # [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) tests on a single
  # file:
  #
  # ```
  # Cmd    Returns   Meaning
  # "A"  | Time    | Last access time for file1
  # "b"  | boolean | True if file1 is a block device
  # "c"  | boolean | True if file1 is a character device
  # "C"  | Time    | Last change time for file1
  # "d"  | boolean | True if file1 exists and is a directory
  # "e"  | boolean | True if file1 exists
  # "f"  | boolean | True if file1 exists and is a regular file
  # "g"  | boolean | True if file1 has the \CF{setgid} bit
  #      |         | set (false under NT)
  # "G"  | boolean | True if file1 exists and has a group
  #      |         | ownership equal to the caller's group
  # "k"  | boolean | True if file1 exists and has the sticky bit set
  # "l"  | boolean | True if file1 exists and is a symbolic link
  # "M"  | Time    | Last modification time for file1
  # "o"  | boolean | True if file1 exists and is owned by
  #      |         | the caller's effective uid
  # "O"  | boolean | True if file1 exists and is owned by
  #      |         | the caller's real uid
  # "p"  | boolean | True if file1 exists and is a fifo
  # "r"  | boolean | True if file1 is readable by the effective
  #      |         | uid/gid of the caller
  # "R"  | boolean | True if file is readable by the real
  #      |         | uid/gid of the caller
  # "s"  | int/nil | If file1 has nonzero size, return the size,
  #      |         | otherwise return nil
  # "S"  | boolean | True if file1 exists and is a socket
  # "u"  | boolean | True if file1 has the setuid bit set
  # "w"  | boolean | True if file1 exists and is writable by
  #      |         | the effective uid/gid
  # "W"  | boolean | True if file1 exists and is writable by
  #      |         | the real uid/gid
  # "x"  | boolean | True if file1 exists and is executable by
  #      |         | the effective uid/gid
  # "X"  | boolean | True if file1 exists and is executable by
  #      |         | the real uid/gid
  # "z"  | boolean | True if file1 exists and has a zero length
  # ```
  #
  # Tests that take two files:
  #
  # ```
  # "-"  | boolean | True if file1 and file2 are identical
  # "="  | boolean | True if the modification times of file1
  #      |         | and file2 are equal
  # "<"  | boolean | True if the modification time of file1
  #      |         | is prior to that of file2
  # ">"  | boolean | True if the modification time of file1
  #      |         | is after that of file2
  # ```
  sig do
    params(
        cmd: String,
        file1: String,
        file2: String,
    )
    .returns(T.any(TrueClass, FalseClass, Time))
  end
  def test(cmd, file1, file2=T.unsafe(nil)); end

  # Transfers control to the end of the active `catch` block waiting for *tag*.
  # Raises `UncaughtThrowError` if there is no `catch` block for the *tag*. The
  # optional second parameter supplies a return value for the `catch` block,
  # which otherwise defaults to `nil`. For examples, see Kernel::catch.
  sig do
    params(
        tag: Object,
        obj: BasicObject,
    )
    .returns(T.noreturn)
  end
  def throw(tag, obj=nil); end

  # Specifies the handling of signals. The first parameter is a signal name (a
  # string such as "SIGALRM", "SIGUSR1", and so on) or a signal number. The
  # characters "SIG" may be omitted from the signal name. The command or block
  # specifies code to be run when the signal is raised. If the command is the
  # string "IGNORE" or "SIG\_IGN", the signal will be ignored. If the command is
  # "DEFAULT" or "SIG\_DFL", the Ruby's default handler will be invoked. If the
  # command is "EXIT", the script will be terminated by the signal. If the
  # command is "SYSTEM\_DEFAULT", the operating system's default handler will be
  # invoked. Otherwise, the given command or block will be run. The special
  # signal name "EXIT" or signal number zero will be invoked just prior to
  # program termination. trap returns the previous handler for the given signal.
  #
  # ```ruby
  # Signal.trap(0, proc { puts "Terminating: #{$$}" })
  # Signal.trap("CLD")  { puts "Child died" }
  # fork && Process.wait
  # ```
  #
  # produces:
  #
  # ```
  # Terminating: 27461
  # Child died
  # Terminating: 27460
  # ```
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
  def trap(signal, command=T.unsafe(nil), &blk); end

  # If warnings have been disabled (for example with the `-W0` flag), does
  # nothing. Otherwise, converts each of the messages to strings, appends a
  # newline character to the string if the string does not end in a newline, and
  # calls
  # [`Warning.warn`](https://docs.ruby-lang.org/en/2.7.0/Warning.html#method-i-warn)
  # with the string.
  #
  # ```
  #   warn("warning 1", "warning 2")
  #
  # <em>produces:</em>
  #
  #   warning 1
  #   warning 2
  # ```
  #
  # If the `uplevel` keyword argument is given, the string will be prepended
  # with information for the given caller frame in the same format used by the
  # `rb_warn` C function.
  #
  # ```
  #   # In baz.rb
  #   def foo
  #     warn("invalid call to foo", uplevel: 1)
  #   end
  #
  #   def bar
  #     foo
  #   end
  #
  #   bar
  #
  # <em>produces:</em>
  #
  #   baz.rb:6: warning: invalid call to foo
  # ```
  #
  # If `category` keyword argument is given, passes the category to
  # `Warning.warn`. The category given must be be one of the following
  # categories:
  #
  # :deprecated
  # :   Used for warning for deprecated functionality that may be removed in the
  #     future.
  # :experimental
  # :   Used for experimental features that may change in future releases.
  sig do
    params(
        msg: String,
    )
    .returns(NilClass)
  end
  def warn(*msg); end

  # With no arguments, raises the exception in `$!` or raises a
  # [`RuntimeError`](https://docs.ruby-lang.org/en/2.7.0/RuntimeError.html) if
  # `$!` is `nil`. With a single `String` argument, raises a `RuntimeError` with
  # the string as a message. Otherwise, the first parameter should be an
  # `Exception` class (or another object that returns an `Exception` object when
  # sent an `exception` message). The optional second parameter sets the message
  # associated with the exception (accessible via
  # [`Exception#message`](https://docs.ruby-lang.org/en/2.7.0/Exception.html#method-i-message)),
  # and the third parameter is an array of callback information (accessible via
  # [`Exception#backtrace`](https://docs.ruby-lang.org/en/2.7.0/Exception.html#method-i-backtrace)).
  # The `cause` of the generated exception (accessible via
  # [`Exception#cause`](https://docs.ruby-lang.org/en/2.7.0/Exception.html#method-i-cause))
  # is automatically set to the "current" exception (`$!`), if any. An
  # alternative value, either an `Exception` object or `nil`, can be specified
  # via the `:cause` argument.
  #
  # Exceptions are caught by the `rescue` clause of `begin...end` blocks.
  #
  # ```ruby
  # raise "Failed to create socket"
  # raise ArgumentError, "No parameters", caller
  # ```
  #
  #
  # Also aliased as:
  # [`fail`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-fail)
  sig {returns(T.noreturn)}
  sig do
    params(
        arg0: T.any(T::Class[T.anything], Exception, String),
    )
    .returns(T.noreturn)
  end
  sig do
    params(
        arg0: T.any(T::Class[T.anything], Exception),
        arg1: T.untyped,
        arg2: T.nilable(T::Array[String]),
    )
    .returns(T.noreturn)
  end
  def raise(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  # Replaces the current process by running the given external *command*, which
  # can take one of the following forms:
  #
  # `exec(commandline)`
  # :   command line string which is passed to the standard shell
  # `exec(cmdname, arg1, ...)`
  # :   command name and one or more arguments (no shell)
  # `exec([cmdname, argv0], arg1, ...)`
  # :   command name, [argv](0) and zero or more arguments (no shell)
  #
  #
  # In the first form, the string is taken as a command line that is subject to
  # shell expansion before being executed.
  #
  # The standard shell always means `"/bin/sh"` on Unix-like systems, otherwise,
  # `ENV["RUBYSHELL"]` or `ENV["COMSPEC"]` on Windows and similar. The command
  # is passed as an argument to the `"-c"` switch to the shell, except in the
  # case of `COMSPEC`.
  #
  # If the string from the first form (`exec("command")`) follows these simple
  # rules:
  #
  # *   no meta characters
  # *   not starting with shell reserved word or special built-in
  # *   Ruby invokes the command directly without shell
  #
  #
  # You can force shell invocation by adding ";" to the string (because ";" is a
  # meta character).
  #
  # Note that this behavior is observable by pid obtained (return value of
  # spawn() and
  # [`IO#pid`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pid) for
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen)) is
  # the pid of the invoked command, not shell.
  #
  # In the second form (`exec("command1", "arg1", ...)`), the first is taken as
  # a command name and the rest are passed as parameters to command with no
  # shell expansion.
  #
  # In the third form (`exec(["command", "argv0"], "arg1", ...)`), starting a
  # two-element array at the beginning of the command, the first element is the
  # command to be executed, and the second argument is used as the `argv[0]`
  # value, which may show up in process listings.
  #
  # In order to execute the command, one of the `exec(2)` system calls are used,
  # so the running command may inherit some of the environment of the original
  # program (including open file descriptors).
  #
  # This behavior is modified by the given `env` and `options` parameters. See
  # ::spawn for details.
  #
  # If the command fails to execute (typically Errno::ENOENT when it was not
  # found) a
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # exception is raised.
  #
  # This method modifies process attributes according to given `options` before
  # `exec(2)` system call. See ::spawn for more details about the given
  # `options`.
  #
  # The modified attributes may be retained when `exec(2)` system call fails.
  #
  # For example, hard resource limits are not restorable.
  #
  # Consider to create a child process using ::spawn or
  # [`Kernel#system`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-system)
  # if this is not acceptable.
  #
  # ```ruby
  # exec "echo *"       # echoes list of files in current directory
  # # never get here
  #
  # exec "echo", "*"    # echoes an asterisk
  # # never get here
  # ```
  sig { params(args: String).returns(T.noreturn) }
  def exec(*args); end

  # Executes *command...* in a subshell. *command...* is one of following forms.
  #
  # `commandline`
  # :   command line string which is passed to the standard shell
  # `cmdname, arg1, ...`
  # :   command name and one or more arguments (no shell)
  # `[cmdname, argv0], arg1, ...`
  # :   command name, `argv[0]` and zero or more arguments (no shell)
  #
  #
  # system returns `true` if the command gives zero exit status, `false` for non
  # zero exit status. Returns `nil` if command execution fails. An error status
  # is available in `$?`.
  #
  # If the `exception: true` argument is passed, the method raises an exception
  # instead of returning `false` or `nil`.
  #
  # The arguments are processed in the same way as for
  # [`Kernel#spawn`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-spawn).
  #
  # The hash arguments, env and options, are same as
  # [`exec`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-exec) and
  # [`spawn`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-spawn).
  # See
  # [`Kernel#spawn`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-spawn)
  # for details.
  #
  # ```ruby
  # system("echo *")
  # system("echo", "*")
  # ```
  #
  # *produces:*
  #
  # ```
  # config.h main.rb
  # *
  # ```
  #
  # Error handling:
  #
  # ```ruby
  # system("cat nonexistent.txt")
  # # => false
  # system("catt nonexistent.txt")
  # # => nil
  #
  # system("cat nonexistent.txt", exception: true)
  # # RuntimeError (Command failed with exit 1: cat)
  # system("catt nonexistent.txt", exception: true)
  # # Errno::ENOENT (No such file or directory - catt)
  # ```
  #
  # See
  # [`Kernel#exec`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-exec)
  # for the standard shell.
  sig do
    params(
      env: T.any(String, [String, String], T::Hash[String, T.nilable(String)]),
      argv0: T.any(String, [String, String]),
      args: String,
      options: T.untyped,
    ).returns(T.nilable(T::Boolean))
  end
  def system(env, argv0 = T.unsafe(nil), *args, **options); end

  # Yields self to the block and returns the result of the block.
  #
  # ```ruby
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
  #   then {|url| URI(url).read }.
  #   then {|response| JSON.parse(response) }
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
  # ```
  #
  # Good usage for `then` is value piping in method chains:
  #
  # ```ruby
  # require 'open-uri'
  # require 'json'
  #
  # construct_url(arguments).
  #   then {|url| URI(url).read }.
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
end
