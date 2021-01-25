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
  # Returns the flag to show the warning messages for category. Supported
  # categories are:
  #
  # :deprecated -- deprecation warnings
  #   - assignment of non-nil value to $, and $;
  #   - keyword arguments
  #   - proc/lambda without block
  #   - etc.
  #
  # :experimental -- experimental features
  #   - patterrn matching
  sig {params(category: Symbol).returns(T::Boolean)}
  def self.[](category); end

  # Sets the warning flags for category. Supported categories are:
  #
  # :deprecated -- deprecation warnings
  #   - assignment of non-nil value to $, and $;
  #   - keyword arguments
  #   - proc/lambda without block
  #   - etc.
  #
  # :experimental -- experimental features
  #   - patterrn matching
  sig {params(category: Symbol, setting: T::Boolean).returns(T::Boolean)}
  def self.[]=(category, setting); end

  # Writes warning message `msg` to $stderr. This method is called by Ruby for
  # all emitted warnings.
  sig {params(msg: Object).returns(NilClass)}
  def warn(msg); end
end
