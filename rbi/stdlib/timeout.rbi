# typed: __STDLIB_INTERNAL

# [`Timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html) long-running
# blocks
#
# ## Synopsis
#
# ```ruby
# require 'timeout'
# status = Timeout::timeout(5) {
#   # Something that should be interrupted if it takes more than 5 seconds...
# }
# ```
#
# ## Description
#
# [`Timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html) provides a way
# to auto-terminate a potentially long-running operation if it hasn't finished
# in a fixed amount of time.
#
# Previous versions didn't use a module for namespacing, however
# [`timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html#method-i-timeout)
# is provided for backwards compatibility. You should prefer
# [`Timeout.timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html#method-c-timeout)
# instead.
#
# ## Copyright
#
# Copyright
# :   (C) 2000  Network Applied Communication Laboratory, Inc.
# Copyright
# :   (C) 2000  Information-technology Promotion Agency, Japan
module Timeout
  # Perform an operation in a block, raising an error if it takes longer than
  # `sec` seconds to complete.
  #
  # `sec`
  # :   Number of seconds to wait for the block to terminate. Any number may be
  #     used, including Floats to specify fractional seconds. A value of 0 or
  #     `nil` will execute the block without any timeout.
  # `klass`
  # :   [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html)
  #     [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) to raise if
  #     the block fails to terminate in `sec` seconds. Omitting will use the
  #     default,
  #     [`Timeout::Error`](https://docs.ruby-lang.org/en/2.7.0/Timeout/Error.html)
  # `message`
  # :   [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html) message to
  #     raise with
  #     [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html)
  #     [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html). Omitting will
  #     use the default, "execution expired"
  #
  #
  # Returns the result of the block **if** the block completed before `sec`
  # seconds, otherwise throws an exception, based on the value of `klass`.
  #
  # The exception thrown to terminate the given block cannot be rescued inside
  # the block unless `klass` is given explicitly. However, the block can use
  # ensure to prevent the handling of the exception. For that reason, this
  # method cannot be relied on to enforce timeouts for untrusted blocks.
  #
  # Note that this is both a method of module
  # [`Timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html), so you can
  # `include Timeout` into your classes so they have a
  # [`timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html#method-i-timeout)
  # method, as well as a module method, so you can call it directly as
  # [`Timeout.timeout()`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html#method-c-timeout).
  sig do
    type_parameters(:U).params(
      sec: T.nilable(Numeric),
      klass: T.any(NilClass, T.class_of(Exception)),
      message: T.untyped,
      blk: T.proc.params(sec: T.nilable(Numeric)).returns(T.type_parameter(:U))
    ).returns(T.type_parameter(:U))
  end
  def self.timeout(sec, klass = nil, message = nil, &blk); end
end

# Raised by
# [`Timeout.timeout`](https://docs.ruby-lang.org/en/2.7.0/Timeout.html#method-c-timeout)
# when the block times out.
class Timeout::Error < RuntimeError
  sig { params(msg: T.untyped, blk: T.proc.params(exc: Timeout::Error).void).void }
  def self.catch(msg = nil, &blk); end

  sig { returns(T.nilable(Thread)) }
  def thread; end

  sig { params(msg: T.untyped).void }
  def initialize(msg = nil)
    @thread = T.let(T.unsafe(nil), T.nilable(Thread))
  end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def exception(*_); end

  sig {returns(::T.untyped)}
  def thread(); end
end
