# typed: __STDLIB_INTERNAL

# Document-class:
# [`TracePoint`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html)
#
# A class that provides the functionality of
# [`Kernel#set_trace_func`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-set_trace_func)
# in a nice Object-Oriented API.
#
# ## Example
#
# We can use [`TracePoint`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html)
# to gather information specifically for exceptions:
#
# ```ruby
# trace = TracePoint.new(:raise) do |tp|
#     p [tp.lineno, tp.event, tp.raised_exception]
# end
# #=> #<TracePoint:disabled>
#
# trace.enable
# #=> false
#
# 0 / 0
# #=> [5, :raise, #<ZeroDivisionError: divided by 0>]
# ```
#
# ## Events
#
# If you don't specify the type of events you want to listen for,
# [`TracePoint`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html) will
# include all available events.
#
# **Note** do not depend on current event set, as this list is subject to
# change. Instead, it is recommended you specify the type of events you want to
# use.
#
# To filter what is traced, you can pass any of the following as `events`:
#
# `:line`
# :   execute code on a new line
# `:class`
# :   start a class or module definition
# `:end`
# :   finish a class or module definition
# `:call`
# :   call a Ruby method
# `:return`
# :   return from a Ruby method
# `:c_call`
# :   call a C-language routine
# `:c_return`
# :   return from a C-language routine
# `:raise`
# :   raise an exception
# `:b_call`
# :   event hook at block entry
# `:b_return`
# :   event hook at block ending
# `:thread_begin`
# :   event hook at thread beginning
# `:thread_end`
# :   event hook at thread ending
# `:fiber_switch`
# :   event hook at fiber switch
# `:script_compiled`
# :   new Ruby code compiled (with `eval`, `load` or `require`)
class TracePoint < Object
  sig do
    params(
        events: Symbol,
        blk: T.proc.params(tp: TracePoint).void,
    )
    .void
  end
  def initialize(*events, &blk); end

  # Returns internal information of
  # [`TracePoint`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html).
  #
  # The contents of the returned value are implementation specific. It may be
  # changed in future.
  #
  # This method is only for debugging
  # [`TracePoint`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html) itself.
  sig {returns(T.untyped)}
  def self.stat; end

  # Document-method: trace
  #
  # ```
  # A convenience method for TracePoint.new, that activates the trace
  # automatically.
  #
  #        trace = TracePoint.trace(:call) { |tp| [tp.lineno, tp.event] }
  #        #=> #<TracePoint:enabled>
  #
  #        trace.enabled? #=> true
  # ```
  sig do
    params(
        events: Symbol,
        blk: T.proc.params(tp: TracePoint).void,
    )
    .returns(TracePoint)
  end
  def self.trace(*events, &blk); end

  # Return the generated binding object from event
  sig {returns(T.untyped)}
  def binding; end

  # Return the called name of the method being called
  sig {returns(T.untyped)}
  def callee_id; end

  # Return class or module of the method being called.
  #
  # ```ruby
  # class C; def foo; end; end
  # trace = TracePoint.new(:call) do |tp|
  #   p tp.defined_class #=> C
  # end.enable do
  #   C.new.foo
  # end
  # ```
  #
  # If method is defined by a module, then that module is returned.
  #
  # ```ruby
  # module M; def foo; end; end
  # class C; include M; end;
  # trace = TracePoint.new(:call) do |tp|
  #   p tp.defined_class #=> M
  # end.enable do
  #   C.new.foo
  # end
  # ```
  #
  # **Note:**
  # [`defined_class`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html#method-i-defined_class)
  # returns singleton class.
  #
  # 6th block parameter of
  # [`Kernel#set_trace_func`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-set_trace_func)
  # passes original class of attached by singleton class.
  #
  # **This is a difference between
  # [`Kernel#set_trace_func`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-set_trace_func)
  # and [`TracePoint`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html).**
  #
  # ```ruby
  # class C; def self.foo; end; end
  # trace = TracePoint.new(:call) do |tp|
  #   p tp.defined_class #=> #<Class:C>
  # end.enable do
  #   C.foo
  # end
  # ```
  sig {returns(T::Module[T.anything])}
  def defined_class; end

  # Deactivates the trace
  #
  # Return true if trace was enabled. Return false if trace was disabled.
  #
  # ```ruby
  # trace.enabled?      #=> true
  # trace.disable       #=> true (previous status)
  # trace.enabled?      #=> false
  # trace.disable       #=> false
  # ```
  #
  # If a block is given, the trace will only be disable within the scope of the
  # block.
  #
  # ```ruby
  # trace.enabled?
  # #=> true
  #
  # trace.disable do
  #     trace.enabled?
  #     # only disabled for this block
  # end
  #
  # trace.enabled?
  # #=> true
  # ```
  #
  # Note: You cannot access event hooks within the block.
  #
  # ```ruby
  # trace.disable { p tp.lineno }
  # #=> RuntimeError: access from outside
  # ```
  sig {returns(T::Boolean)}
  sig {params(blk: T.proc.void).void}
  def disable(&blk); end

  # Activates the trace.
  #
  # Returns `true` if trace was enabled. Returns `false` if trace was disabled.
  #
  # ```ruby
  # trace.enabled?  #=> false
  # trace.enable    #=> false (previous state)
  #                 #   trace is enabled
  # trace.enabled?  #=> true
  # trace.enable    #=> true (previous state)
  #                 #   trace is still enabled
  # ```
  #
  # If a block is given, the trace will only be enabled within the scope of the
  # block.
  #
  # ```ruby
  # trace.enabled?
  # #=> false
  #
  # trace.enable do
  #   trace.enabled?
  #   # only enabled for this block
  # end
  #
  # trace.enabled?
  # #=> false
  # ```
  #
  # `target`, `target_line` and `target_thread` parameters are used to limit
  # tracing only to specified code objects. `target` should be a code object for
  # which
  # [`RubyVM::InstructionSequence.of`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-of)
  # will return an instruction sequence.
  #
  # ```ruby
  # t = TracePoint.new(:line) { |tp| p tp }
  #
  # def m1
  #   p 1
  # end
  #
  # def m2
  #   p 2
  # end
  #
  # t.enable(target: method(:m1))
  #
  # m1
  # # prints #<TracePoint:line@test.rb:5 in `m1'>
  # m2
  # # prints nothing
  # ```
  #
  # Note: You cannot access event hooks within the `enable` block.
  #
  # ```ruby
  # trace.enable { p tp.lineno }
  # #=> RuntimeError: access from outside
  # ```
  sig {returns(T::Boolean)}
  sig {params(blk: T.proc.void).void}
  def enable(&blk); end

  # The current status of the trace
  sig {returns(T::Boolean)}
  def enabled?; end

  # Type of event
  #
  # See [Events at
  # `TracePoint`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html#class-TracePoint-label-Events)
  # for more information.
  def event; end

  # Return a string containing a human-readable
  # [`TracePoint`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html) status.
  sig {returns(String)}
  def inspect; end

  # Line number of the event
  sig {returns(Integer)}
  def lineno; end

  # Return the name at the definition of the method being called
  sig {returns(T.untyped)}
  def method_id; end

  # Path of the file being run
  sig {returns(String)}
  def path; end

  # Value from exception raised on the `:raise` event
  sig {returns(T.untyped)}
  def raised_exception; end

  # Return value from `:return`, `c_return`, and `b_return` event
  sig {returns(T.untyped)}
  def return_value; end

  # Return the trace object during event
  #
  # Same as
  # [`TracePoint#binding`](https://docs.ruby-lang.org/en/2.7.0/TracePoint.html#method-i-binding):
  #
  # ```ruby
  # trace.binding.eval('self')
  # ```
  sig {returns(T.untyped)}
  def self; end
end
