# typed: __STDLIB_INTERNAL

# The [`Warning`](https://docs.ruby-lang.org/en/2.6.0/Warning.html) module
# contains a single method named
# [`warn`](https://docs.ruby-lang.org/en/2.6.0/Warning.html#method-i-warn), and
# the module extends itself, making `Warning.warn` available.
# [`Warning.warn`](https://docs.ruby-lang.org/en/2.6.0/Warning.html#method-i-warn)
# is called for all warnings issued by Ruby. By default, warnings are printed to
# $stderr.
#
# By overriding
# [`Warning.warn`](https://docs.ruby-lang.org/en/2.6.0/Warning.html#method-i-warn),
# you can change how warnings are handled by Ruby, either filtering some
# warnings, and/or outputting warnings somewhere other than $stderr. When
# [`Warning.warn`](https://docs.ruby-lang.org/en/2.6.0/Warning.html#method-i-warn)
# is overridden, super can be called to get the default behavior of printing the
# warning to $stderr.
module Warning
  # Writes warning message `msg` to $stderr. This method is called by Ruby for
  # all emitted warnings.
  def warn(msg); end
end
