# typed: __STDLIB_INTERNAL

# `ARGF` is a stream designed for use in scripts that process files given as
# command-line arguments or passed in via STDIN.
#
# The arguments passed to your script are stored in the `ARGV`
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html), one argument per
# element. `ARGF` assumes that any arguments that aren't filenames have been
# removed from `ARGV`. For example:
#
# ```
# $ ruby argf.rb --verbose file1 file2
#
# ARGV  #=> ["--verbose", "file1", "file2"]
# option = ARGV.shift #=> "--verbose"
# ARGV  #=> ["file1", "file2"]
# ```
#
# You can now use `ARGF` to work with a concatenation of each of these named
# files. For instance, `ARGF.read` will return the contents of *file1* followed
# by the contents of *file2*.
#
# After a file in `ARGV` has been read `ARGF` removes it from the
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html). Thus, after all
# files have been read `ARGV` will be empty.
#
# You can manipulate `ARGV` yourself to control what `ARGF` operates on. If you
# remove a file from `ARGV`, it is ignored by `ARGF`; if you add files to
# `ARGV`, they are treated as if they were named on the command line. For
# example:
#
# ```ruby
# ARGV.replace ["file1"]
# ARGF.readlines # Returns the contents of file1 as an Array
# ARGV           #=> []
# ARGV.replace ["file2", "file3"]
# ARGF.read      # Returns the contents of file2 and file3
# ```
#
# If `ARGV` is empty, `ARGF` acts as if it contained STDIN, i.e. the data piped
# to your script. For example:
#
# ```
# $ echo "glark" | ruby -e 'p ARGF.read'
# "glark\n"
# ```
::ARGF = T.let(T.unsafe(nil), Object)
# [`ARGV`](https://docs.ruby-lang.org/en/2.6.0/Object.html#ARGV) contains the
# command line arguments used to run ruby.
#
# A library like
# [`OptionParser`](https://docs.ruby-lang.org/en/2.6.0/OptionParser.html) can be
# used to process command-line arguments.
::ARGV = T.let(T.unsafe(nil), Array)
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
# The SVN revision for this ruby.
::RUBY_REVISION = T.let(T.unsafe(nil), Integer)
# The running version of ruby
::RUBY_VERSION = T.let(T.unsafe(nil), String)
# Holds the original stderr
::STDERR = T.let(T.unsafe(nil), IO)
# Holds the original stdin
::STDIN = T.let(T.unsafe(nil), IO)
# Holds the original stdout
::STDOUT = T.let(T.unsafe(nil), IO)
# The [`Binding`](https://docs.ruby-lang.org/en/2.6.0/Binding.html) of the top
# level scope
::TOPLEVEL_BINDING = T.let(T.unsafe(nil), Binding)
# An obsolete alias of `true`
::TRUE = T.let(T.unsafe(nil), TrueClass)
