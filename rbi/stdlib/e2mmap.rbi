# typed: __STDLIB_INTERNAL

# Helper module for easily defining exceptions with predefined messages.
#
# ## Usage
#
# 1.
#
# ```
# class Foo
#   extend Exception2MessageMapper
#   def_e2message ExistingExceptionClass, "message..."
#   def_exception :NewExceptionClass, "message..."[, superclass]
#   ...
# end
# ```
#
# 2.
#
# ```
# module Error
#   extend Exception2MessageMapper
#   def_e2message ExistingExceptionClass, "message..."
#   def_exception :NewExceptionClass, "message..."[, superclass]
#   ...
# end
# class Foo
#   include Error
#   ...
# end
#
# foo = Foo.new
# foo.Fail ....
# ```
#
# 3.
#
# ```
# module Error
#   extend Exception2MessageMapper
#   def_e2message ExistingExceptionClass, "message..."
#   def_exception :NewExceptionClass, "message..."[, superclass]
#   ...
# end
# class Foo
#   extend Exception2MessageMapper
#   include Error
#   ...
# end
#
# Foo.Fail NewExceptionClass, arg...
# Foo.Fail ExistingExceptionClass, arg...
# ```
module Exception2MessageMapper
  # Fail(err, \*rest)
  #
  # ```
  # err:    exception
  # rest:   message arguments
  # ```
  #
  #
  # Also aliased as:
  # [`Fail`](https://docs.ruby-lang.org/en/2.6.0/Exception2MessageMapper.html#method-i-Fail),
  # [`fail`](https://docs.ruby-lang.org/en/2.6.0/Exception2MessageMapper.html#method-i-fail)
  sig {params(err: Exception, rest: T.untyped).void}
  def Raise(err, *rest); end

  # Alias for:
  # [`Raise`](https://docs.ruby-lang.org/en/2.6.0/Exception2MessageMapper.html#method-i-Raise)
  sig {params(err: Exception, rest: T.untyped).void}
  def fail(err, *rest); end

  # Alias for:
  # [`Raise`](https://docs.ruby-lang.org/en/2.6.0/Exception2MessageMapper.html#method-i-Raise)
  sig {params(err: Exception, rest: T.untyped).void}
  def Fail(err, *rest); end

  # [`def_e2message`](https://docs.ruby-lang.org/en/2.6.0/Exception2MessageMapper.html#method-i-def_e2message)(c,
  # m)
  #
  # ```
  #     c:  exception
  #     m:  message_form
  # define exception c with message m.
  # ```
  sig {params(c: T::Class[T.anything], m: String).void}
  def def_e2message(c, m); end

  # [`def_exception`](https://docs.ruby-lang.org/en/2.6.0/Exception2MessageMapper.html#method-i-def_exception)(n,
  # m, s)
  #
  # ```
  #     n:  exception_name
  #     m:  message_form
  #     s:  superclass(default: StandardError)
  # define exception named ``c'' with message m.
  # ```
  sig {params(n: Symbol, m: String, s: T::Class[T.anything]).void}
  def def_exception(n, m, s = StandardError); end
end
