# typed: __STDLIB_INTERNAL

# The [`GetoptLong`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html) class
# allows you to parse command line options similarly to the GNU getopt\_long() C
# library call. Note, however, that
# [`GetoptLong`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html) is a pure
# Ruby implementation.
#
# [`GetoptLong`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html) allows for
# POSIX-style options like `--file` as well as single letter options like `-f`
#
# The empty option `--` (two minus symbols) is used to end option processing.
# This can be particularly important if options have optional arguments.
#
# Here is a simple example of usage:
#
# ```ruby
# require 'getoptlong'
#
# opts = GetoptLong.new(
#   [ '--help', '-h', GetoptLong::NO_ARGUMENT ],
#   [ '--repeat', '-n', GetoptLong::REQUIRED_ARGUMENT ],
#   [ '--name', GetoptLong::OPTIONAL_ARGUMENT ]
# )
#
# dir = nil
# name = nil
# repetitions = 1
# opts.each do |opt, arg|
#   case opt
#     when '--help'
#       puts <<-EOF
# hello [OPTION] ... DIR
#
# -h, --help:
#    show help
#
# --repeat x, -n x:
#    repeat x times
#
# --name [name]:
#    greet user by name, if name not supplied default is John
#
# DIR: The directory in which to issue the greeting.
#       EOF
#     when '--repeat'
#       repetitions = arg.to_i
#     when '--name'
#       if arg == ''
#         name = 'John'
#       else
#         name = arg
#       end
#   end
# end
#
# if ARGV.length != 1
#   puts "Missing dir argument (try --help)"
#   exit 0
# end
#
# dir = ARGV.shift
#
# Dir.chdir(dir)
# for i in (1..repetitions)
#   print "Hello"
#   if name
#     print ", #{name}"
#   end
#   puts
# end
# ```
#
# Example command line:
#
# ```
# hello -n 6 --name -- /tmp
# ```
class GetoptLong

  # Argument flags.
  ARGUMENT_FLAGS = T.let(T.unsafe(nil), T::Array[String])

  # Orderings.
  ORDERINGS = T.let(T.unsafe(nil), T::Array[String])

  STATUS_TERMINATED = T.let(T.unsafe(nil), Integer)

  VERSION = T.let(T.unsafe(nil), String)

  def initialize(*arguments); end

  # Iterator version of 'get'.
  #
  # The block is called repeatedly with two arguments: The first is the option
  # name. The second is the argument which followed it (if any). Example:
  # ('--opt', 'value')
  #
  # The option name is always converted to the first (preferred) name given in
  # the original options to
  # [`GetoptLong.new`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html#method-c-new).
  #
  # Also aliased as:
  # [`each_option`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html#method-i-each_option)
  def each; end

  # 'each\_option' is an alias of 'each'.
  #
  # Alias for:
  # [`each`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html#method-i-each)
  def each_option; end

  # Examine whether an option processing is failed.
  def error; end

  # Examine whether an option processing is failed.
  def error?; end

  # Return the appropriate error message in POSIX-defined format. If no error
  # has occurred, returns nil.
  def error_message; end

  # Get next option name and its argument, as an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of two elements.
  #
  # The option name is always converted to the first (preferred) name given in
  # the original options to
  # [`GetoptLong.new`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html#method-c-new).
  #
  # Example: ['--option', 'value']
  #
  # Returns nil if the processing is complete (as determined by
  # [`STATUS_TERMINATED`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html#STATUS_TERMINATED)).
  #
  # Also aliased as:
  # [`get_option`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html#method-i-get_option)
  def get; end

  # 'get\_option' is an alias of 'get'.
  #
  # Alias for:
  # [`get`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html#method-i-get)
  def get_option; end

  # Return ordering.
  def ordering; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the handling of the
  # ordering of options and arguments. A
  # [`RuntimeError`](https://docs.ruby-lang.org/en/2.6.0/RuntimeError.html) is
  # raised if option processing has already started.
  #
  # The supplied value must be a member of
  # [`GetoptLong::ORDERINGS`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html#ORDERINGS).
  # It alters the processing of options as follows:
  #
  # **REQUIRE\_ORDER** :
  #
  # Options are required to occur before non-options.
  #
  # Processing of options ends as soon as a word is encountered that has not
  # been preceded by an appropriate option flag.
  #
  # For example, if -a and -b are options which do not take arguments, parsing
  # command line arguments of '-a one -b two' would result in 'one', '-b', 'two'
  # being left in ARGV, and only ('-a', '') being processed as an option/arg
  # pair.
  #
  # This is the default ordering, if the environment variable POSIXLY\_CORRECT
  # is set. (This is for compatibility with GNU getopt\_long.)
  #
  # **PERMUTE** :
  #
  # Options can occur anywhere in the command line parsed. This is the default
  # behavior.
  #
  # Every sequence of words which can be interpreted as an option (with or
  # without argument) is treated as an option; non-option words are skipped.
  #
  # For example, if -a does not require an argument and -b optionally takes an
  # argument, parsing '-a one -b two three' would result in ('-a',") and ('-b',
  # 'two') being processed as option/arg pairs, and 'one','three' being left in
  # ARGV.
  #
  # If the ordering is set to PERMUTE but the environment variable
  # POSIXLY\_CORRECT is set, REQUIRE\_ORDER is used instead. This is for
  # compatibility with GNU getopt\_long.
  #
  # **RETURN\_IN\_ORDER** :
  #
  # All words on the command line are processed as options. Words not preceded
  # by a short or long option flag are passed as arguments with an option of ''
  # (empty string).
  #
  # For example, if -a requires an argument but -b does not, a command line of
  # '-a one -b two three' would result in option/arg pairs of ('-a', 'one')
  # ('-b', ''), (", 'two'), (", 'three') being processed.
  def ordering=(ordering); end

  # Set/Unset 'quiet' mode.
  def quiet; end

  # Set/Unset 'quiet' mode.
  def quiet=(_); end

  # Set/Unset 'quiet' mode.
  def quiet?; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) options. Takes the
  # same argument as
  # [`GetoptLong.new`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong.html#method-c-new).
  #
  # Raises a
  # [`RuntimeError`](https://docs.ruby-lang.org/en/2.6.0/RuntimeError.html) if
  # option processing has already started.
  def set_options(*arguments); end

  # Explicitly terminate option processing.
  def terminate; end

  # Returns true if option processing has terminated, false otherwise.
  def terminated?; end

  protected

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) an error (a protected
  # method).
  def set_error(type, message); end
end

class GetoptLong::AmbiguousOption < ::GetoptLong::Error; end

# [`Error`](https://docs.ruby-lang.org/en/2.6.0/GetoptLong/Error.html) types.
class GetoptLong::Error < ::StandardError; end

class GetoptLong::InvalidOption < ::GetoptLong::Error; end

class GetoptLong::MissingArgument < ::GetoptLong::Error; end

class GetoptLong::NeedlessArgument < ::GetoptLong::Error; end
