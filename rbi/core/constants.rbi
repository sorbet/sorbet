# typed: __STDLIB_INTERNAL

# [`ARGV`](https://docs.ruby-lang.org/en/2.7.0/Object.html#ARGV) contains the
# command line arguments used to run ruby.
#
# A library like
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) can be
# used to process command-line arguments.
::ARGV = T.let(T.unsafe(nil), T::Array[T.untyped])
::CROSS_COMPILING = T.let(T.unsafe(nil), NilClass)
# An obsolete alias of `false`
::FALSE = T.let(T.unsafe(nil), FalseClass)
# An obsolete alias of `nil`
::NIL = T.let(T.unsafe(nil), NilClass)
# The copyright string for ruby
::RUBY_COPYRIGHT = T.let(T.unsafe(nil), String)
# The full ruby version string, like `ruby -v` prints
::RUBY_DESCRIPTION = T.let(T.unsafe(nil), String)
# The engine or interpreter this ruby uses.
::RUBY_ENGINE = T.let(T.unsafe(nil), String)
# The version of the engine or interpreter this ruby uses.
::RUBY_ENGINE_VERSION = T.let(T.unsafe(nil), String)
# The patchlevel for this ruby. If this is a development build of ruby the
# patchlevel will be -1
::RUBY_PATCHLEVEL = T.let(T.unsafe(nil), Integer)
# The platform for this ruby
::RUBY_PLATFORM = T.let(T.unsafe(nil), String)
# The date this ruby was released
::RUBY_RELEASE_DATE = T.let(T.unsafe(nil), String)
# The GIT commit hash for this ruby.
::RUBY_REVISION = T.let(T.unsafe(nil), Integer)
# The running version of ruby
::RUBY_VERSION = T.let(T.unsafe(nil), String)
# Holds the original stderr
::STDERR = T.let(T.unsafe(nil), IO)
# Holds the original stdin
::STDIN = T.let(T.unsafe(nil), IO)
# Holds the original stdout
::STDOUT = T.let(T.unsafe(nil), IO)
# The [`Binding`](https://docs.ruby-lang.org/en/2.7.0/Binding.html) of the top
# level scope
::TOPLEVEL_BINDING = T.let(T.unsafe(nil), Binding)
# An obsolete alias of `true`
::TRUE = T.let(T.unsafe(nil), TrueClass)
# Constant that is available if using the __END__ keyword
::DATA = T.let(T.unsafe(nil), IO)
