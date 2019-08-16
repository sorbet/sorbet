# typed: __STDLIB_INTERNAL

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
  # [TracePoint](TracePoint.downloaded.ruby_doc) .
  # 
  # The contents of the returned value are implementation specific. It may
  # be changed in future.
  # 
  # This method is only for debugging
  # [TracePoint](TracePoint.downloaded.ruby_doc) itself.
  sig {returns(T.untyped)}
  def self.stat; end

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

  sig {returns(Module)}
  def defined_class; end

  # Deactivates the trace
  # 
  # Return true if trace was enabled. Return false if trace was disabled.
  # 
  # ```ruby
  # trace.enabled?       #=> true
  # trace.disable        #=> true (previous status)
  # trace.enabled?       #=> false
  # trace.disable        #=> false
  # ```
  # 
  # If a block is given, the trace will only be disable within the scope of
  # the block.
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

  sig {returns(T::Boolean)}
  sig {params(blk: T.proc.void).void}
  def enable(&blk); end

  # The current status of the trace
  sig {returns(T::Boolean)}
  def enabled?; end

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
  # Same as [\#binding](TracePoint.downloaded.ruby_doc#method-i-binding) :
  # 
  # ```ruby
  # trace.binding.eval('self')
  # ```
  sig {returns(T.untyped)}
  def self; end
end
