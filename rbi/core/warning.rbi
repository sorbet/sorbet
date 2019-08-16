# typed: __STDLIB_INTERNAL

# The [Warning](Warning) module contains a single
# method named [warn](Warning#method-i-warn) , and the
# module extends itself, making `Warning.warn` available.
# [\#warn](Warning#method-i-warn) is called for all
# warnings issued by Ruby. By default, warnings are printed to $stderr.
#
# By overriding [\#warn](Warning#method-i-warn) , you
# can change how warnings are handled by Ruby, either filtering some
# warnings, and/or outputting warnings somewhere other than $stderr. When
# [\#warn](Warning#method-i-warn) is overridden, super
# can be called to get the default behavior of printing the warning to
# $stderr.
module Warning
end
