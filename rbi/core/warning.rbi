# typed: __STDLIB_INTERNAL

# The [`Warning`](https://docs.ruby-lang.org/en/2.7.0/Warning.html) module
# contains a single method named
# [`warn`](https://docs.ruby-lang.org/en/2.7.0/Warning.html#method-i-warn), and
# the module extends itself, making
# [`Warning.warn`](https://docs.ruby-lang.org/en/2.7.0/Warning.html#method-i-warn)
# available.
# [`Warning.warn`](https://docs.ruby-lang.org/en/2.7.0/Warning.html#method-i-warn)
# is called for all warnings issued by Ruby. By default, warnings are printed to
# $stderr.
#
# Changing the behavior of
# [`Warning.warn`](https://docs.ruby-lang.org/en/2.7.0/Warning.html#method-i-warn)
# is useful to customize how warnings are handled by Ruby, for instance by
# filtering some warnings, and/or outputting warnings somewhere other than
# $stderr.
#
# If you want to change the behavior of
# [`Warning.warn`](https://docs.ruby-lang.org/en/2.7.0/Warning.html#method-i-warn)
# you should use +Warning.extend(MyNewModuleWithWarnMethod)+ and you can use
# `super` to get the default behavior of printing the warning to $stderr.
#
# Example:
#
# ```ruby
# module MyWarningFilter
#   def warn(message, category: nil, **kwargs)
#     if /some warning I want to ignore/.match?(message)
#       # ignore
#     else
#       super
#     end
#   end
# end
# Warning.extend MyWarningFilter
# ```
#
# You should never redefine
# [`Warning#warn`](https://docs.ruby-lang.org/en/2.7.0/Warning.html#method-i-warn)
# (the instance method), as that will then no longer provide a way to use the
# default behavior.
#
# The `warning` gem provides convenient ways to customize
# [`Warning.warn`](https://docs.ruby-lang.org/en/2.7.0/Warning.html#method-i-warn).
module Warning
  # Returns the flag to show the warning messages for `category`. Supported
  # categories are:
  #
  # `:deprecated`
  # :   deprecation warnings
  #
  # *   assignment of non-nil value to `$,` and `$;`
  # *   keyword arguments
  # *   proc/lambda without block
  #
  # etc.
  #
  # `:experimental`
  # :   experimental features
  #
  # *   Pattern matching
  sig {params(category: Symbol).returns(T::Boolean)}
  def self.[](category); end

  # Sets the warning flags for `category`. See
  # [`Warning.[]`](https://docs.ruby-lang.org/en/2.7.0/Warning.html#method-c-5B-5D)
  # for the categories.
  sig {params(category: Symbol, setting: T::Boolean).returns(T::Boolean)}
  def self.[]=(category, setting); end

  # Writes warning message `msg` to $stderr. This method is called by Ruby for
  # all emitted warnings. A `category` may be included with the warning.
  #
  # See the documentation of the
  # [`Warning`](https://docs.ruby-lang.org/en/2.7.0/Warning.html) module for how
  # to customize this.
  sig {params(msg: Object, category: T.nilable(T.any(String, Symbol))).returns(NilClass)}
  def warn(msg, category: nil); end
end
