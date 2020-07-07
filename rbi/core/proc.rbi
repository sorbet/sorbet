# typed: __STDLIB_INTERNAL

# A `Proc` object is an encapsulation of a block of code, which can be stored in
# a local variable, passed to a method or another
# [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html), and can be called.
# [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) is an essential
# concept in Ruby and a core of its functional programming features.
#
# ```ruby
# square = Proc.new {|x| x**2 }
#
# square.call(3)  #=> 9
# # shorthands:
# square.(3)      #=> 9
# square[3]       #=> 9
# ```
#
# [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) objects are
# *closures*, meaning they remember and can use the entire context in which they
# were created.
#
# ```ruby
# def gen_times(factor)
#   Proc.new {|n| n*factor } # remembers the value of factor at the moment of creation
# end
#
# times3 = gen_times(3)
# times5 = gen_times(5)
#
# times3.call(12)               #=> 36
# times5.call(5)                #=> 25
# times3.call(times5.call(4))   #=> 60
# ```
#
# ## Creation
#
# There are several methods to create a
# [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html)
#
# *   Use the [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) class
#     constructor:
#
# ```ruby
# proc1 = Proc.new {|x| x**2 }
# ```
#
# *   Use the
#     [`Kernel#proc`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-proc)
#     method as a shorthand of
#     [`Proc.new`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-c-new):
#
# ```ruby
# proc2 = proc {|x| x**2 }
# ```
#
# *   Receiving a block of code into proc argument (note the `&`):
#
# ```ruby
# def make_proc(&block)
#   block
# end
#
# proc3 = make_proc {|x| x**2 }
# ```
#
# *   Construct a proc with lambda semantics using the
#     [`Kernel#lambda`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-lambda)
#     method (see below for explanations about lambdas):
#
# ```ruby
# lambda1 = lambda {|x| x**2 }
# ```
#
# *   Use the Lambda literal syntax (also constructs a proc with lambda
#     semantics):
#
# ```ruby
# lambda2 = ->(x) { x**2 }
# ```
#
#
# ## Lambda and non-lambda semantics
#
# Procs are coming in two flavors: lambda and non-lambda (regular procs).
# Differences are:
#
# *   In lambdas, `return` means exit from this lambda;
# *   In regular procs, `return` means exit from embracing method (and will
#     throw `LocalJumpError` if invoked outside the method);
# *   In lambdas, arguments are treated in the same way as in methods: strict,
#     with `ArgumentError` for mismatching argument number, and no additional
#     argument processing;
# *   Regular procs accept arguments more generously: missing arguments are
#     filled with `nil`, single
#     [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) arguments are
#     deconstructed if the proc has multiple arguments, and there is no error
#     raised on extra arguments.
#
#
# Examples:
#
# ```ruby
# p = proc {|x, y| "x=#{x}, y=#{y}" }
# p.call(1, 2)      #=> "x=1, y=2"
# p.call([1, 2])    #=> "x=1, y=2", array deconstructed
# p.call(1, 2, 8)   #=> "x=1, y=2", extra argument discarded
# p.call(1)         #=> "x=1, y=", nil substituted instead of error
#
# l = lambda {|x, y| "x=#{x}, y=#{y}" }
# l.call(1, 2)      #=> "x=1, y=2"
# l.call([1, 2])    # ArgumentError: wrong number of arguments (given 1, expected 2)
# l.call(1, 2, 8)   # ArgumentError: wrong number of arguments (given 3, expected 2)
# l.call(1)         # ArgumentError: wrong number of arguments (given 1, expected 2)
#
# def test_return
#   -> { return 3 }.call      # just returns from lambda into method body
#   proc { return 4 }.call    # returns from method
#   return 5
# end
#
# test_return # => 4, return from proc
# ```
#
# Lambdas are useful as self-sufficient functions, in particular useful as
# arguments to higher-order functions, behaving exactly like Ruby methods.
#
# Procs are useful for implementing iterators:
#
# ```ruby
# def test
#   [[1, 2], [3, 4], [5, 6]].map {|a, b| return a if a + b > 10 }
#                             #  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# end
# ```
#
# Inside `map`, the block of code is treated as a regular (non-lambda) proc,
# which means that the internal arrays will be deconstructed to pairs of
# arguments, and `return` will exit from the method `test`. That would not be
# possible with a stricter lambda.
#
# You can tell a lambda from a regular proc by using the
# [`lambda?`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-i-lambda-3F)
# instance method.
#
# Lambda semantics is typically preserved during the proc lifetime, including
# `&`-deconstruction to a block of code:
#
# ```ruby
# p = proc {|x, y| x }
# l = lambda {|x, y| x }
# [[1, 2], [3, 4]].map(&p) #=> [1, 2]
# [[1, 2], [3, 4]].map(&l) # ArgumentError: wrong number of arguments (given 1, expected 2)
# ```
#
# The only exception is dynamic method definition: even if defined by passing a
# non-lambda proc, methods still have normal semantics of argument checking.
#
# ```ruby
# class C
#   define_method(:e, &proc {})
# end
# C.new.e(1,2)       #=> ArgumentError
# C.new.method(:e).to_proc.lambda?   #=> true
# ```
#
# This exception ensures that methods never have unusual argument passing
# conventions, and makes it easy to have wrappers defining methods that behave
# as usual.
#
# ```ruby
# class C
#   def self.def2(name, &body)
#     define_method(name, &body)
#   end
#
#   def2(:f) {}
# end
# C.new.f(1,2)       #=> ArgumentError
# ```
#
# The wrapper *def2* receives `body` as a non-lambda proc, yet defines a method
# which has normal semantics.
#
# ## Conversion of other objects to procs
#
# Any object that implements the `to_proc` method can be converted into a proc
# by the `&` operator, and therefore con be consumed by iterators.
#
# ```ruby
# class Greater
#   def initialize(greating)
#     @greating = greating
#   end
#
#   def to_proc
#     proc {|name| "#{@greating}, #{name}!" }
#   end
# end
#
# hi = Greater.new("Hi")
# hey = Greater.new("Hey")
# ["Bob", "Jane"].map(&hi)    #=> ["Hi, Bob!", "Hi, Jane!"]
# ["Bob", "Jane"].map(&hey)   #=> ["Hey, Bob!", "Hey, Jane!"]
# ```
#
# Of the Ruby core classes, this method is implemented by
# [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html),
# [`Method`](https://docs.ruby-lang.org/en/2.6.0/Method.html), and
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html).
#
# ```ruby
# :to_s.to_proc.call(1)           #=> "1"
# [1, 2].map(&:to_s)              #=> ["1", "2"]
#
# method(:puts).to_proc.call(1)   # prints 1
# [1, 2].each(&method(:puts))     # prints 1, 2
#
# {test: 1}.to_proc.call(:test)       #=> 1
# %i[test many keys].map(&{test: 1})  #=> [1, nil, nil]
# ```
class Proc < Object
  # Creates a new `Proc` object, bound to the current context. `Proc::new` may
  # be called without a block only within a method with an attached block, in
  # which case that block is converted to the `Proc` object.
  #
  # ```ruby
  # def proc_from
  #   Proc.new
  # end
  # proc = proc_from { "hello" }
  # proc.call   #=> "hello"
  # ```
  def self.new(*_); end

  # Invokes the block with `obj` as the proc's parameter like
  # [`Proc#call`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-i-call).
  # This allows a proc object to be the target of a `when` clause in a case
  # statement.
  def ===(*_); end

  # Returns the number of mandatory arguments. If the block is declared to take
  # no arguments, returns 0. If the block is known to take exactly n arguments,
  # returns n. If the block has optional arguments, returns -n-1, where n is the
  # number of mandatory arguments, with the exception for blocks that are not
  # lambdas and have only a finite number of optional arguments; in this latter
  # case, returns n. Keyword arguments will be considered as a single additional
  # argument, that argument being mandatory if any keyword argument is
  # mandatory. A `proc` with no argument declarations is the same as a block
  # declaring `||` as its arguments.
  #
  # ```ruby
  # proc {}.arity                  #=>  0
  # proc { || }.arity              #=>  0
  # proc { |a| }.arity             #=>  1
  # proc { |a, b| }.arity          #=>  2
  # proc { |a, b, c| }.arity       #=>  3
  # proc { |*a| }.arity            #=> -1
  # proc { |a, *b| }.arity         #=> -2
  # proc { |a, *b, c| }.arity      #=> -3
  # proc { |x:, y:, z:0| }.arity   #=>  1
  # proc { |*a, x:, y:0| }.arity   #=> -2
  #
  # proc   { |a=0| }.arity         #=>  0
  # lambda { |a=0| }.arity         #=> -1
  # proc   { |a=0, b| }.arity      #=>  1
  # lambda { |a=0, b| }.arity      #=> -2
  # proc   { |a=0, b=0| }.arity    #=>  0
  # lambda { |a=0, b=0| }.arity    #=> -1
  # proc   { |a, b=0| }.arity      #=>  1
  # lambda { |a, b=0| }.arity      #=> -2
  # proc   { |(a, b), c=0| }.arity #=>  1
  # lambda { |(a, b), c=0| }.arity #=> -2
  # proc   { |a, x:0, y:0| }.arity #=>  1
  # lambda { |a, x:0, y:0| }.arity #=> -2
  # ```
  sig {returns(Integer)}
  def arity(); end

  # Returns the binding associated with *prc*.
  #
  # ```ruby
  # def fred(param)
  #   proc {}
  # end
  #
  # b = fred(99)
  # eval("param", b.binding)   #=> 99
  # ```
  sig {returns(Binding)}
  def binding(); end

  # Invokes the block, setting the block's parameters to the values in *params*
  # using something close to method calling semantics. Returns the value of the
  # last expression evaluated in the block.
  #
  # ```ruby
  # a_proc = Proc.new {|scalar, *values| values.map {|value| value*scalar } }
  # a_proc.call(9, 1, 2, 3)    #=> [9, 18, 27]
  # a_proc[9, 1, 2, 3]         #=> [9, 18, 27]
  # a_proc.(9, 1, 2, 3)        #=> [9, 18, 27]
  # a_proc.yield(9, 1, 2, 3)   #=> [9, 18, 27]
  # ```
  #
  # Note that `prc.()` invokes `prc.call()` with the parameters given. It's
  # syntactic sugar to hide "call".
  #
  # For procs created using `lambda` or `->()` an error is generated if the
  # wrong number of parameters are passed to the proc. For procs created using
  # `Proc.new` or `Kernel.proc`, extra parameters are silently discarded and
  # missing parameters are set to `nil`.
  #
  # ```ruby
  # a_proc = proc {|a,b| [a,b] }
  # a_proc.call(1)   #=> [1, nil]
  #
  # a_proc = lambda {|a,b| [a,b] }
  # a_proc.call(1)   # ArgumentError: wrong number of arguments (given 1, expected 2)
  # ```
  #
  # See also
  # [`Proc#lambda?`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-i-lambda-3F).
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.untyped)
  end
  def call(*arg0); end

  # Invokes the block, setting the block's parameters to the values in *params*
  # using something close to method calling semantics. Returns the value of the
  # last expression evaluated in the block.
  #
  # ```ruby
  # a_proc = Proc.new {|scalar, *values| values.map {|value| value*scalar } }
  # a_proc.call(9, 1, 2, 3)    #=> [9, 18, 27]
  # a_proc[9, 1, 2, 3]         #=> [9, 18, 27]
  # a_proc.(9, 1, 2, 3)        #=> [9, 18, 27]
  # a_proc.yield(9, 1, 2, 3)   #=> [9, 18, 27]
  # ```
  #
  # Note that `prc.()` invokes `prc.call()` with the parameters given. It's
  # syntactic sugar to hide "call".
  #
  # For procs created using `lambda` or `->()` an error is generated if the
  # wrong number of parameters are passed to the proc. For procs created using
  # `Proc.new` or `Kernel.proc`, extra parameters are silently discarded and
  # missing parameters are set to `nil`.
  #
  # ```ruby
  # a_proc = proc {|a,b| [a,b] }
  # a_proc.call(1)   #=> [1, nil]
  #
  # a_proc = lambda {|a,b| [a,b] }
  # a_proc.call(1)   # ArgumentError: wrong number of arguments (given 1, expected 2)
  # ```
  #
  # See also
  # [`Proc#lambda?`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-i-lambda-3F).
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.untyped)
  end
  def [](*arg0); end

  # Returns a curried proc. If the optional *arity* argument is given, it
  # determines the number of arguments. A curried proc receives some arguments.
  # If a sufficient number of arguments are supplied, it passes the supplied
  # arguments to the original proc and returns the result. Otherwise, returns
  # another curried proc that takes the rest of arguments.
  #
  # ```ruby
  # b = proc {|x, y, z| (x||0) + (y||0) + (z||0) }
  # p b.curry[1][2][3]           #=> 6
  # p b.curry[1, 2][3, 4]        #=> 6
  # p b.curry(5)[1][2][3][4][5]  #=> 6
  # p b.curry(5)[1, 2][3, 4][5]  #=> 6
  # p b.curry(1)[1]              #=> 1
  #
  # b = proc {|x, y, z, *w| (x||0) + (y||0) + (z||0) + w.inject(0, &:+) }
  # p b.curry[1][2][3]           #=> 6
  # p b.curry[1, 2][3, 4]        #=> 10
  # p b.curry(5)[1][2][3][4][5]  #=> 15
  # p b.curry(5)[1, 2][3, 4][5]  #=> 15
  # p b.curry(1)[1]              #=> 1
  #
  # b = lambda {|x, y, z| (x||0) + (y||0) + (z||0) }
  # p b.curry[1][2][3]           #=> 6
  # p b.curry[1, 2][3, 4]        #=> wrong number of arguments (given 4, expected 3)
  # p b.curry(5)                 #=> wrong number of arguments (given 5, expected 3)
  # p b.curry(1)                 #=> wrong number of arguments (given 1, expected 3)
  #
  # b = lambda {|x, y, z, *w| (x||0) + (y||0) + (z||0) + w.inject(0, &:+) }
  # p b.curry[1][2][3]           #=> 6
  # p b.curry[1, 2][3, 4]        #=> 10
  # p b.curry(5)[1][2][3][4][5]  #=> 15
  # p b.curry(5)[1, 2][3, 4][5]  #=> 15
  # p b.curry(1)                 #=> wrong number of arguments (given 1, expected 3)
  #
  # b = proc { :foo }
  # p b.curry[]                  #=> :foo
  # ```
  sig do
    params(
        arity: Integer,
    )
    .returns(Proc)
  end
  def curry(arity=T.unsafe(nil)); end

  # Returns a hash value corresponding to proc body.
  #
  # See also Object#hash.
  sig {returns(Integer)}
  def hash(); end

  # Returns `true` for a [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html)
  # object for which argument handling is rigid. Such procs are typically
  # generated by `lambda`.
  #
  # A [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object generated
  # by `proc` ignores extra arguments.
  #
  # ```ruby
  # proc {|a,b| [a,b] }.call(1,2,3)    #=> [1,2]
  # ```
  #
  # It provides `nil` for missing arguments.
  #
  # ```ruby
  # proc {|a,b| [a,b] }.call(1)        #=> [1,nil]
  # ```
  #
  # It expands a single array argument.
  #
  # ```ruby
  # proc {|a,b| [a,b] }.call([1,2])    #=> [1,2]
  # ```
  #
  # A [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object generated
  # by `lambda` doesn't have such tricks.
  #
  # ```ruby
  # lambda {|a,b| [a,b] }.call(1,2,3)  #=> ArgumentError
  # lambda {|a,b| [a,b] }.call(1)      #=> ArgumentError
  # lambda {|a,b| [a,b] }.call([1,2])  #=> ArgumentError
  # ```
  #
  # [`Proc#lambda?`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-i-lambda-3F)
  # is a predicate for the tricks. It returns `true` if no tricks apply.
  #
  # ```ruby
  # lambda {}.lambda?            #=> true
  # proc {}.lambda?              #=> false
  # ```
  #
  # [`Proc.new`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-c-new) is
  # the same as `proc`.
  #
  # ```ruby
  # Proc.new {}.lambda?          #=> false
  # ```
  #
  # `lambda`, `proc` and
  # [`Proc.new`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-c-new)
  # preserve the tricks of a
  # [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object given by `&`
  # argument.
  #
  # ```ruby
  # lambda(&lambda {}).lambda?   #=> true
  # proc(&lambda {}).lambda?     #=> true
  # Proc.new(&lambda {}).lambda? #=> true
  #
  # lambda(&proc {}).lambda?     #=> false
  # proc(&proc {}).lambda?       #=> false
  # Proc.new(&proc {}).lambda?   #=> false
  # ```
  #
  # A [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object generated
  # by `&` argument has the tricks
  #
  # ```ruby
  # def n(&b) b.lambda? end
  # n {}                         #=> false
  # ```
  #
  # The `&` argument preserves the tricks if a
  # [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object is given by
  # `&` argument.
  #
  # ```ruby
  # n(&lambda {})                #=> true
  # n(&proc {})                  #=> false
  # n(&Proc.new {})              #=> false
  # ```
  #
  # A [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object converted
  # from a method has no tricks.
  #
  # ```ruby
  # def m() end
  # method(:m).to_proc.lambda?   #=> true
  #
  # n(&method(:m))               #=> true
  # n(&method(:m).to_proc)       #=> true
  # ```
  #
  # `define_method` is treated the same as method definition. The defined method
  # has no tricks.
  #
  # ```ruby
  # class C
  #   define_method(:d) {}
  # end
  # C.new.d(1,2)       #=> ArgumentError
  # C.new.method(:d).to_proc.lambda?   #=> true
  # ```
  #
  # `define_method` always defines a method without the tricks, even if a
  # non-lambda [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object is
  # given. This is the only exception for which the tricks are not preserved.
  #
  # ```ruby
  # class C
  #   define_method(:e, &proc {})
  # end
  # C.new.e(1,2)       #=> ArgumentError
  # C.new.method(:e).to_proc.lambda?   #=> true
  # ```
  #
  # This exception ensures that methods never have tricks and makes it easy to
  # have wrappers to define methods that behave as usual.
  #
  # ```ruby
  # class C
  #   def self.def2(name, &body)
  #     define_method(name, &body)
  #   end
  #
  #   def2(:f) {}
  # end
  # C.new.f(1,2)       #=> ArgumentError
  # ```
  #
  # The wrapper *def2* defines a method which has no tricks.
  sig {returns(T::Boolean)}
  def lambda?(); end

  # Returns the parameter information of this proc.
  #
  # ```ruby
  # prc = lambda{|x, y=42, *other|}
  # prc.parameters  #=> [[:req, :x], [:opt, :y], [:rest, :other]]
  # ```
  sig {returns(T::Array[[Symbol, Symbol]])}
  def parameters(); end

  # Marks the proc as passing keywords through a normal argument splat.
  # This should only be called on procs that accept an argument splat (`*args`)
  # but not explicit keywords or a keyword splat. It marks the proc such that if
  # the proc is called with keyword arguments, the final hash argument is marked
  # with a special flag such that if it is the final element of a normal argument
  # splat to another method call, and that method call does not include explicit
  # keywords or a keyword splat, the final element is interpreted as keywords.
  # In other words, keywords will be passed through the proc to other methods.
  #
  # This should only be used for procs that delegate keywords to another method,
  # and only for backwards compatibility with Ruby versions before 2.7.
  #
  # This method will probably be removed at some point, as it exists only for
  # backwards compatibility. As it does not exist in Ruby versions before 2.7,
  # check that the proc responds to this method before calling it. Also, be
  # aware that if this method is removed, the behavior of the proc will change
  # so that it does not pass through keywords.
  #
  # ```ruby
  #
  # module Mod
  #   foo = ->(meth, *args, &block) do
  #     send(:"do_#{meth}", *args, &block)
  #   end
  #   foo.ruby2_keywords if foo.respond_to?(:ruby2_keywords)
  # end
  # ```
  sig { returns(T.self_type) }
  def ruby2_keywords; end

  # Returns the Ruby source filename and line number containing this proc or
  # `nil` if this proc was not defined in Ruby (i.e. native).
  sig {returns([String, Integer])}
  def source_location(); end

  # Part of the protocol for converting objects to `Proc` objects. Instances of
  # class `Proc` simply return themselves.
  sig {returns(T.self_type)}
  def to_proc(); end

  # Returns the unique identifier for this proc, along with an indication of
  # where the proc was defined.
  #
  # Also aliased as:
  # [`inspect`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-i-inspect)
  sig {returns(String)}
  def to_s(); end

  # Alias for:
  # [`to_s`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-i-to_s)
  sig {returns(String)}
  def inspect(); end

  # Invokes the block, setting the block's parameters to the values in *params*
  # using something close to method calling semantics. Returns the value of the
  # last expression evaluated in the block.
  #
  # ```ruby
  # a_proc = Proc.new {|scalar, *values| values.map {|value| value*scalar } }
  # a_proc.call(9, 1, 2, 3)    #=> [9, 18, 27]
  # a_proc[9, 1, 2, 3]         #=> [9, 18, 27]
  # a_proc.(9, 1, 2, 3)        #=> [9, 18, 27]
  # a_proc.yield(9, 1, 2, 3)   #=> [9, 18, 27]
  # ```
  #
  # Note that `prc.()` invokes `prc.call()` with the parameters given. It's
  # syntactic sugar to hide "call".
  #
  # For procs created using `lambda` or `->()` an error is generated if the
  # wrong number of parameters are passed to the proc. For procs created using
  # `Proc.new` or `Kernel.proc`, extra parameters are silently discarded and
  # missing parameters are set to `nil`.
  #
  # ```ruby
  # a_proc = proc {|a,b| [a,b] }
  # a_proc.call(1)   #=> [1, nil]
  #
  # a_proc = lambda {|a,b| [a,b] }
  # a_proc.call(1)   # ArgumentError: wrong number of arguments (given 1, expected 2)
  # ```
  #
  # See also
  # [`Proc#lambda?`](https://docs.ruby-lang.org/en/2.6.0/Proc.html#method-i-lambda-3F).
  def yield(*_); end
end
