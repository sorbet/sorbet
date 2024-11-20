# typed: __STDLIB_INTERNAL

# Objects of class [`Binding`](https://docs.ruby-lang.org/en/2.7.0/Binding.html)
# encapsulate the execution context at some particular place in the code and
# retain this context for future use. The variables, methods, value of `self`,
# and possibly an iterator block that can be accessed in this context are all
# retained. [`Binding`](https://docs.ruby-lang.org/en/2.7.0/Binding.html)
# objects can be created using
# [`Kernel#binding`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-binding),
# and are made available to the callback of
# [`Kernel#set_trace_func`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-set_trace_func)
# and instances of
# [`TracePoint`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html).
#
# These binding objects can be passed as the second argument of the
# [`Kernel#eval`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-eval)
# method, establishing an environment for the evaluation.
#
# ```ruby
# class Demo
#   def initialize(n)
#     @secret = n
#   end
#   def get_binding
#     binding
#   end
# end
#
# k1 = Demo.new(99)
# b1 = k1.get_binding
# k2 = Demo.new(-3)
# b2 = k2.get_binding
#
# eval("@secret", b1)   #=> 99
# eval("@secret", b2)   #=> -3
# eval("@secret")       #=> nil
# ```
#
# [`Binding`](https://docs.ruby-lang.org/en/2.7.0/Binding.html) objects have no
# class-specific methods.
class Binding < Object
  # Evaluates the Ruby expression(s) in *string*, in the *binding*'s context. If
  # the optional *filename* and *lineno* parameters are present, they will be
  # used when reporting syntax errors.
  #
  # ```ruby
  # def get_binding(param)
  #   binding
  # end
  # b = get_binding("hello")
  # b.eval("param")   #=> "hello"
  # ```
  def eval(*_); end

  # Returns `true` if a local variable `symbol` exists.
  #
  # ```ruby
  # def foo
  #   a = 1
  #   binding.local_variable_defined?(:a) #=> true
  #   binding.local_variable_defined?(:b) #=> false
  # end
  # ```
  #
  # This method is the short version of the following code:
  #
  # ```ruby
  # binding.eval("defined?(#{symbol}) == 'local-variable'")
  # ```
  sig {params(symbol: T.any(String, Symbol)).returns(T::Boolean)}
  def local_variable_defined?(symbol); end

  # Returns the value of the local variable `symbol`.
  #
  # ```ruby
  # def foo
  #   a = 1
  #   binding.local_variable_get(:a) #=> 1
  #   binding.local_variable_get(:b) #=> NameError
  # end
  # ```
  #
  # This method is the short version of the following code:
  #
  # ```ruby
  # binding.eval("#{symbol}")
  # ```
  sig {params(symbol: T.any(String, Symbol)).returns(T.untyped)}
  def local_variable_get(symbol); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) local variable named
  # `symbol` as `obj`.
  #
  # ```ruby
  # def foo
  #   a = 1
  #   bind = binding
  #   bind.local_variable_set(:a, 2) # set existing local variable `a'
  #   bind.local_variable_set(:b, 3) # create new local variable `b'
  #                                  # `b' exists only in binding
  #
  #   p bind.local_variable_get(:a)  #=> 2
  #   p bind.local_variable_get(:b)  #=> 3
  #   p a                            #=> 2
  #   p b                            #=> NameError
  # end
  # ```
  #
  # This method behaves similarly to the following code:
  #
  # ```ruby
  # binding.eval("#{symbol} = #{obj}")
  # ```
  #
  # if `obj` can be dumped in Ruby code.
  sig {params(symbol: T.any(String, Symbol), obj: T.untyped).returns(T.untyped)}
  def local_variable_set(symbol, obj); end

  # Returns the names of the binding's local variables as symbols.
  #
  # ```ruby
  # def foo
  #   a = 1
  #   2.times do |n|
  #     binding.local_variables #=> [:a, :n]
  #   end
  # end
  # ```
  #
  # This method is the short version of the following code:
  #
  # ```ruby
  # binding.eval("local_variables")
  # ```
  def local_variables; end

  # Returns the bound receiver of the binding object.
  sig {returns(Object)}
  def receiver(); end

  # Returns the Ruby source filename and line number of the binding object.
  sig {returns([String, Integer])}
  def source_location(); end

  # Opens an IRB session where +binding.irb+ is called which allows for
  # interactive debugging. You can call any methods or variables available in
  # the current scope, and mutate state if you need to.
  #
  #
  # Given a Ruby file called +potato.rb+ containing the following code:
  #
  #     class Potato
  #       def initialize
  #         @cooked = false
  #         binding.irb
  #         puts "Cooked potato: #{@cooked}"
  #       end
  #     end
  #
  #     Potato.new
  #
  # Running <code>ruby potato.rb</code> will open an IRB session where
  # +binding.irb+ is called, and you will see the following:
  #
  #     $ ruby potato.rb
  #
  #     From: potato.rb @ line 4 :
  #
  #         1: class Potato
  #         2:   def initialize
  #         3:     @cooked = false
  #      => 4:     binding.irb
  #         5:     puts "Cooked potato: #{@cooked}"
  #         6:   end
  #         7: end
  #         8:
  #         9: Potato.new
  #
  #     irb(#<Potato:0x00007feea1916670>):001:0>
  #
  # You can type any valid Ruby code and it will be evaluated in the current
  # context. This allows you to debug without having to run your code repeatedly:
  #
  #     irb(#<Potato:0x00007feea1916670>):001:0> @cooked
  #     => false
  #     irb(#<Potato:0x00007feea1916670>):002:0> self.class
  #     => Potato
  #     irb(#<Potato:0x00007feea1916670>):003:0> caller.first
  #     => ".../2.5.1/lib/ruby/2.5.0/irb/workspace.rb:85:in `eval'"
  #     irb(#<Potato:0x00007feea1916670>):004:0> @cooked = true
  #     => true
  #
  # You can exit the IRB session with the +exit+ command. Note that exiting will
  # resume execution where +binding.irb+ had paused it, as you can see from the
  # output printed to standard output in this example:
  #
  #     irb(#<Potato:0x00007feea1916670>):005:0> exit
  #     Cooked potato: true
  #
  # See IRB for more information.
  sig {params(show_code: T::Boolean).void}
  def irb(show_code: true); end
end
