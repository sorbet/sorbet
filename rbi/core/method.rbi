# typed: __STDLIB_INTERNAL

class Method < Object
  # Returns a `Proc` object corresponding to this method.
  sig {returns(Proc)}
  def to_proc; end

  # Invokes the *meth* with the specified arguments, returning the method’s
  # return value.
  #
  # ```ruby
  # m = 12.method("+")
  # m.call(3)    #=> 15
  # m.call(20)   #=> 32
  # ```
  sig {params(args: T.untyped).returns(T.untyped)}
  def call(*args);end

  # Returns a proc that is the composition of this method and the given *g*
  # . The returned proc takes a variable number of arguments, calls *g* with
  # them then calls this method with the result.
  #
  # ```ruby
  # def f(x)
  #   x * x
  # end
  #
  # f = self.method(:f)
  # g = proc {|x| x + x }
  # p (f << g).call(2) #=> 16
  # ```
  sig {params(g: T.untyped).returns(T.untyped)}
  def <<(g); end

  # Invokes the method with `obj` as the parameter like
  # [call](Method.downloaded.ruby_doc#method-i-call) . This allows a method
  # object to be the target of a `when` clause in a case statement.
  #
  # ```ruby
  # require 'prime'
  #
  # case 1373
  # when Prime.method(:prime?)
  #   # ...
  # end
  # ```
  sig {params(obj: T.untyped).returns(T.untyped)}
  def ===(*obj); end

  # Returns a proc that is the composition of this method and the given *g*
  # . The returned proc takes a variable number of arguments, calls *g* with
  # them then calls this method with the result.
  #
  # ```ruby
  # def f(x)
  #   x * x
  # end
  #
  # f = self.method(:f)
  # g = proc {|x| x + x }
  # p (f >> g).call(2) #=> 8
  # ```
  sig {params(g: T.untyped).returns(T.untyped)}
  def >>(g); end

  # Invokes the *meth* with the specified arguments, returning the method’s
  # return value.
  #
  # ```ruby
  # m = 12.method("+")
  # m.call(3)    #=> 15
  # m.call(20)   #=> 32
  # ```
  sig {params(args: T.untyped).returns(T.untyped)}
  def [](*args); end

  # Returns an indication of the number of arguments accepted by a method.
  # Returns a nonnegative integer for methods that take a fixed number of
  # arguments. For Ruby methods that take a variable number of arguments,
  # returns -n-1, where n is the number of required arguments. Keyword
  # arguments will be considered as a single additional argument, that
  # argument being mandatory if any keyword argument is mandatory. For
  # methods written in C, returns -1 if the call takes a variable number of
  # arguments.
  #
  #     class C
  #       def one;    end
  #       def two(a); end
  #       def three(*a);  end
  #       def four(a, b); end
  #       def five(a, b, *c);    end
  #       def six(a, b, *c, &d); end
  #       def seven(a, b, x:0); end
  #       def eight(x:, y:); end
  #       def nine(x:, y:, **z); end
  #       def ten(*a, x:, y:); end
  #     end
  #     c = C.new
  #     c.method(:one).arity     #=> 0
  #     c.method(:two).arity     #=> 1
  #     c.method(:three).arity   #=> -1
  #     c.method(:four).arity    #=> 2
  #     c.method(:five).arity    #=> -3
  #     c.method(:six).arity     #=> -3
  #     c.method(:seven).arity   #=> -3
  #     c.method(:eight).arity   #=> 1
  #     c.method(:nine).arity    #=> 1
  #     c.method(:ten).arity     #=> -2
  #
  #     "cat".method(:size).arity      #=> 0
  #     "cat".method(:replace).arity   #=> 1
  #     "cat".method(:squeeze).arity   #=> -1
  #     "cat".method(:count).arity     #=> -1
  sig {returns(Integer)}
  def arity; end

  # Returns a clone of this method.
  #
  # ```ruby
  # class A
  #   def foo
  #     return "bar"
  #   end
  # end
  #
  # m = A.new.method(:foo)
  # m.call # => "bar"
  # n = m.clone.call # => "bar"
  # ```
  sig {returns(Method)}
  def clone; end

  sig {params(args: T.untyped).returns(T.untyped)}
  def curry(*args); end

  # Returns the name of the method.
  sig {returns(Symbol)}
  def name; end

  # Returns the original name of the method.
  #
  # ```ruby
  # class C
  #   def foo; end
  #   alias bar foo
  # end
  # C.instance_method(:bar).original_name # => :foo
  # ```
  sig {returns(Symbol)}
  def original_name; end

  # Returns the class or module that defines the method. See also receiver.
  #
  # ```ruby
  # (1..3).method(:map).owner #=> Enumerable
  # ```
  sig {returns(T.any(Class, Module))}
  def owner; end

  # Returns the parameter information of this method.
  #
  # ```ruby
  # def foo(bar); end
  # method(:foo).parameters #=> [[:req, :bar]]
  #
  # def foo(bar, baz, bat, &blk); end
  # method(:foo).parameters #=> [[:req, :bar], [:req, :baz], [:req, :bat], [:block, :blk]]
  #
  # def foo(bar, *args); end
  # method(:foo).parameters #=> [[:req, :bar], [:rest, :args]]
  #
  # def foo(bar, baz, *args, &blk); end
  # method(:foo).parameters #=> [[:req, :bar], [:req, :baz], [:rest, :args], [:block, :blk]]
  # ```
  sig {returns(T::Array[T.untyped])}
  def parameters; end

  # Returns the bound receiver of the method object.
  #
  # ```ruby
  # (1..3).method(:map).receiver # => 1..3
  # ```
  sig {returns(T.untyped)}
  def receiver; end

  # Returns the Ruby source filename and line number containing this method
  # or nil if this method was not defined in Ruby (i.e. native).
  sig {returns(T.untyped)}
  def source_location; end

  # Returns a [Method](Method.downloaded.ruby_doc) of superclass which would
  # be called when super is used or nil if there is no method on superclass.
  sig {returns(T.nilable(Method))}
  def super_method; end

  # Dissociates *meth* from its current receiver. The resulting
  # `UnboundMethod` can subsequently be bound to a new object of the same
  # class (see `UnboundMethod` ).
  sig {returns(T.untyped)}
  def unbind; end
end
