# typed: __STDLIB_INTERNAL
# ## [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
#
# ### Introduction
#
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) is a
# class for command-line option analysis. It is much more advanced, yet also
# easier to use, than
# [`GetoptLong`](https://docs.ruby-lang.org/en/2.7.0/GetoptLong.html), and is a
# more Ruby-oriented solution.
#
# ### Features
#
# 1.  The argument specification and the code to handle it are written in the
#     same place.
# 2.  It can output an option summary; you don't need to maintain this string
#     separately.
# 3.  Optional and mandatory arguments are specified very gracefully.
# 4.  Arguments can be automatically converted to a specified class.
# 5.  Arguments can be restricted to a certain set.
#
#
# All of these features are demonstrated in the examples below. See
# [`make_switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-make_switch)
# for full documentation.
#
# ### Minimal example
#
# ```ruby
# require 'optparse'
#
# options = {}
# OptionParser.new do |opts|
#   opts.banner = "Usage: example.rb [options]"
#
#   opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
#     options[:verbose] = v
#   end
# end.parse!
#
# p options
# p ARGV
# ```
#
# ### Generating Help
#
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) can be
# used to automatically generate help for the commands you write:
#
# ```ruby
# require 'optparse'
#
# Options = Struct.new(:name)
#
# class Parser
#   def self.parse(options)
#     args = Options.new("world")
#
#     opt_parser = OptionParser.new do |opts|
#       opts.banner = "Usage: example.rb [options]"
#
#       opts.on("-nNAME", "--name=NAME", "Name to say hello to") do |n|
#         args.name = n
#       end
#
#       opts.on("-h", "--help", "Prints this help") do
#         puts opts
#         exit
#       end
#     end
#
#     opt_parser.parse!(options)
#     return args
#   end
# end
# options = Parser.parse %w[--help]
#
# #=>
#    # Usage: example.rb [options]
#    #     -n, --name=NAME                  Name to say hello to
#    #     -h, --help                       Prints this help
# ```
#
# ### Required Arguments
#
# For options that require an argument, option specification strings may include
# an option name in all caps. If an option is used without the required
# argument, an exception will be raised.
#
# ```ruby
# require 'optparse'
#
# options = {}
# OptionParser.new do |parser|
#   parser.on("-r", "--require LIBRARY",
#             "Require the LIBRARY before executing your script") do |lib|
#     puts "You required #{lib}!"
#   end
# end.parse!
# ```
#
# Used:
#
# ```
# $ ruby optparse-test.rb -r
# optparse-test.rb:9:in `<main>': missing argument: -r (OptionParser::MissingArgument)
# $ ruby optparse-test.rb -r my-library
# You required my-library!
# ```
#
# ### Type Coercion
#
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
# supports the ability to coerce command line arguments into objects for us.
#
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) comes
# with a few ready-to-use kinds of  type coercion. They are:
#
# *   [`Date`](https://docs.ruby-lang.org/en/2.7.0/Date.html)  -- Anything
#     accepted by `Date.parse`
# *   [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) --
#     Anything accepted by `DateTime.parse`
# *   [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) -- Anything
#     accepted by `Time.httpdate` or `Time.parse`
# *   [`URI`](https://docs.ruby-lang.org/en/2.7.0/URI.html)  -- Anything
#     accepted by `URI.parse`
# *   [`Shellwords`](https://docs.ruby-lang.org/en/2.7.0/Shellwords.html) --
#     Anything accepted by `Shellwords.shellwords`
# *   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) -- Any
#     non-empty string
# *   [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) -- Any
#     integer. Will convert octal. (e.g. 124, -3, 040)
# *   [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) -- Any float.
#     (e.g. 10, 3.14, -100E+13)
# *   [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) -- Any
#     integer, float, or rational (1, 3.4, 1/3)
# *   [`DecimalInteger`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#DecimalInteger)
#     -- Like `Integer`, but no octal format.
# *   [`OctalInteger`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#OctalInteger)
#     -- Like `Integer`, but no decimal format.
# *   [`DecimalNumeric`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#DecimalNumeric)
#     -- Decimal integer or float.
# *   [`TrueClass`](https://docs.ruby-lang.org/en/2.7.0/TrueClass.html) --
#     Accepts '+, yes, true, -, no, false' and defaults as `true`
# *   [`FalseClass`](https://docs.ruby-lang.org/en/2.7.0/FalseClass.html) --
#     Same as `TrueClass`, but defaults to `false`
# *   [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) -- Strings
#     separated by ',' (e.g. 1,2,3)
# *   [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) -- Regular
#     expressions. Also includes options.
#
#
# We can also add our own coercions, which we will cover below.
#
# #### Using Built-in Conversions
#
# As an example, the built-in `Time` conversion is used. The other built-in
# conversions behave in the same way.
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) will
# attempt to parse the argument as a `Time`. If it succeeds, that time will be
# passed to the handler block. Otherwise, an exception will be raised.
#
# ```ruby
# require 'optparse'
# require 'optparse/time'
# OptionParser.new do |parser|
#   parser.on("-t", "--time [TIME]", Time, "Begin execution at given time") do |time|
#     p time
#   end
# end.parse!
# ```
#
# Used:
#
# ```
# $ ruby optparse-test.rb  -t nonsense
# ... invalid argument: -t nonsense (OptionParser::InvalidArgument)
# $ ruby optparse-test.rb  -t 10-11-12
# 2010-11-12 00:00:00 -0500
# $ ruby optparse-test.rb  -t 9:30
# 2014-08-13 09:30:00 -0400
# ```
#
# #### Creating Custom Conversions
#
# The `accept` method on
# [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) may be
# used to create converters. It specifies which conversion block to call
# whenever a class is specified. The example below uses it to fetch a `User`
# object before the `on` handler receives it.
#
# ```ruby
# require 'optparse'
#
# User = Struct.new(:id, :name)
#
# def find_user id
#   not_found = ->{ raise "No User Found for id #{id}" }
#   [ User.new(1, "Sam"),
#     User.new(2, "Gandalf") ].find(not_found) do |u|
#     u.id == id
#   end
# end
#
# op = OptionParser.new
# op.accept(User) do |user_id|
#   find_user user_id.to_i
# end
#
# op.on("--user ID", User) do |user|
#   puts user
# end
#
# op.parse!
# ```
#
# Used:
#
# ```
# $ ruby optparse-test.rb --user 1
# #<struct User id=1, name="Sam">
# $ ruby optparse-test.rb --user 2
# #<struct User id=2, name="Gandalf">
# $ ruby optparse-test.rb --user 3
# optparse-test.rb:15:in `block in find_user': No User Found for id 3 (RuntimeError)
# ```
#
# ### Store options to a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
#
# The `into` option of `order`, `parse` and so on methods stores command line
# options into a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html).
#
# ```ruby
# require 'optparse'
#
# params = {}
# OptionParser.new do |opts|
#   opts.on('-a')
#   opts.on('-b NUM', Integer)
#   opts.on('-v', '--verbose')
# end.parse!(into: params)
#
# p params
# ```
#
# Used:
#
# ```
# $ ruby optparse-test.rb -a
# {:a=>true}
# $ ruby optparse-test.rb -a -v
# {:a=>true, :verbose=>true}
# $ ruby optparse-test.rb -a -b 100
# {:a=>true, :b=>100}
# ```
#
# ### Complete example
#
# The following example is a complete Ruby program. You can run it and see the
# effect of specifying various options. This is probably the best way to learn
# the features of `optparse`.
#
# ```ruby
# require 'optparse'
# require 'optparse/time'
# require 'ostruct'
# require 'pp'
#
# class OptparseExample
#   Version = '1.0.0'
#
#   CODES = %w[iso-2022-jp shift_jis euc-jp utf8 binary]
#   CODE_ALIASES = { "jis" => "iso-2022-jp", "sjis" => "shift_jis" }
#
#   class ScriptOptions
#     attr_accessor :library, :inplace, :encoding, :transfer_type,
#                   :verbose, :extension, :delay, :time, :record_separator,
#                   :list
#
#     def initialize
#       self.library = []
#       self.inplace = false
#       self.encoding = "utf8"
#       self.transfer_type = :auto
#       self.verbose = false
#     end
#
#     def define_options(parser)
#       parser.banner = "Usage: example.rb [options]"
#       parser.separator ""
#       parser.separator "Specific options:"
#
#       # add additional options
#       perform_inplace_option(parser)
#       delay_execution_option(parser)
#       execute_at_time_option(parser)
#       specify_record_separator_option(parser)
#       list_example_option(parser)
#       specify_encoding_option(parser)
#       optional_option_argument_with_keyword_completion_option(parser)
#       boolean_verbose_option(parser)
#
#       parser.separator ""
#       parser.separator "Common options:"
#       # No argument, shows at tail.  This will print an options summary.
#       # Try it and see!
#       parser.on_tail("-h", "--help", "Show this message") do
#         puts parser
#         exit
#       end
#       # Another typical switch to print the version.
#       parser.on_tail("--version", "Show version") do
#         puts Version
#         exit
#       end
#     end
#
#     def perform_inplace_option(parser)
#       # Specifies an optional option argument
#       parser.on("-i", "--inplace [EXTENSION]",
#                 "Edit ARGV files in place",
#                 "(make backup if EXTENSION supplied)") do |ext|
#         self.inplace = true
#         self.extension = ext || ''
#         self.extension.sub!(/\A\.?(?=.)/, ".")  # Ensure extension begins with dot.
#       end
#     end
#
#     def delay_execution_option(parser)
#       # Cast 'delay' argument to a Float.
#       parser.on("--delay N", Float, "Delay N seconds before executing") do |n|
#         self.delay = n
#       end
#     end
#
#     def execute_at_time_option(parser)
#       # Cast 'time' argument to a Time object.
#       parser.on("-t", "--time [TIME]", Time, "Begin execution at given time") do |time|
#         self.time = time
#       end
#     end
#
#     def specify_record_separator_option(parser)
#       # Cast to octal integer.
#       parser.on("-F", "--irs [OCTAL]", OptionParser::OctalInteger,
#                 "Specify record separator (default \\0)") do |rs|
#         self.record_separator = rs
#       end
#     end
#
#     def list_example_option(parser)
#       # List of arguments.
#       parser.on("--list x,y,z", Array, "Example 'list' of arguments") do |list|
#         self.list = list
#       end
#     end
#
#     def specify_encoding_option(parser)
#       # Keyword completion.  We are specifying a specific set of arguments (CODES
#       # and CODE_ALIASES - notice the latter is a Hash), and the user may provide
#       # the shortest unambiguous text.
#       code_list = (CODE_ALIASES.keys + CODES).join(', ')
#       parser.on("--code CODE", CODES, CODE_ALIASES, "Select encoding",
#                 "(#{code_list})") do |encoding|
#         self.encoding = encoding
#       end
#     end
#
#     def optional_option_argument_with_keyword_completion_option(parser)
#       # Optional '--type' option argument with keyword completion.
#       parser.on("--type [TYPE]", [:text, :binary, :auto],
#                 "Select transfer type (text, binary, auto)") do |t|
#         self.transfer_type = t
#       end
#     end
#
#     def boolean_verbose_option(parser)
#       # Boolean switch.
#       parser.on("-v", "--[no-]verbose", "Run verbosely") do |v|
#         self.verbose = v
#       end
#     end
#   end
#
#   #
#   # Return a structure describing the options.
#   #
#   def parse(args)
#     # The options specified on the command line will be collected in
#     # *options*.
#
#     @options = ScriptOptions.new
#     @args = OptionParser.new do |parser|
#       @options.define_options(parser)
#       parser.parse!(args)
#     end
#     @options
#   end
#
#   attr_reader :parser, :options
# end  # class OptparseExample
#
# example = OptparseExample.new
# options = example.parse(ARGV)
# pp options # example.options
# pp ARGV
# ```
#
# ### Shell Completion
#
# For modern shells (e.g. bash, zsh, etc.), you can use shell completion for
# command line options.
#
# ### Further documentation
#
# The above examples should be enough to learn how to use this class. If you
# have any questions, file a ticket at http://bugs.ruby-lang.org.
class OptionParser
  ArgumentStyle = T.let(nil, T.untyped)
  COMPSYS_HEADER = T.let(nil, T.untyped)
  # Decimal integer format, to be converted to
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  DecimalInteger = T.let(nil, T.untyped)
  # Decimal integer/float number format, to be converted to
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) for integer
  # format, [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) for float
  # format.
  DecimalNumeric = T.let(nil, T.untyped)
  DefaultList = T.let(nil, T.untyped)
  NO_ARGUMENT = T.let(nil, T.untyped)
  NoArgument = T.let(nil, T.untyped)
  OPTIONAL_ARGUMENT = T.let(nil, T.untyped)
  # Ruby/C like octal/hexadecimal/binary integer format, to be converted to
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  OctalInteger = T.let(nil, T.untyped)
  Officious = T.let(nil, T.untyped)
  OptionalArgument = T.let(nil, T.untyped)
  REQUIRED_ARGUMENT = T.let(nil, T.untyped)
  RequiredArgument = T.let(nil, T.untyped)
  SPLAT_PROC = T.let(nil, T.untyped)

  sig {params(to: T.untyped, name: T.untyped).returns(T.untyped)}
  def compsys(to, name = File.basename($0)); end

  # Initializes a new instance and evaluates the optional block in context of
  # the instance. Arguments `args` are passed to
  # [`new`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-new),
  # see there for description of parameters.
  #
  # This method is **deprecated**, its behavior corresponds to the older
  # [`new`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-new)
  # method.
  sig {params(args: T.untyped, block: T.untyped).returns(T.untyped)}
  def self.with(*args, &block); end

  # Returns an incremented value of `default` according to `arg`.
  sig {params(arg: T.untyped, default: T.untyped).returns(T.untyped)}
  def self.inc(arg, default = nil); end

  sig {params(args: T.untyped).returns(T.untyped)}
  def inc(*args); end

  sig {params(banner: T.untyped, width: T.untyped, indent: T.untyped, blk: T.nilable(T.proc.params(opts: OptionParser).void)).void}
  def initialize(banner = nil, width = 32, indent = ' ' * 4, &blk); end

  sig {returns(T.untyped)}
  def add_officious(); end

  # Terminates option parsing. Optional parameter `arg` is a string pushed back
  # to be the first non-option argument.
  sig {params(arg: T.untyped).returns(T.untyped)}
  def terminate(arg = nil); end

  sig {params(arg: T.untyped).returns(T.untyped)}
  def self.terminate(arg = nil); end

  sig {returns(T.untyped)}
  def self.top(); end

  # Directs to accept specified class `t`. The argument string is passed to the
  # block in which it should be converted to the desired class.
  #
  # `t`
  # :   Argument class specifier, any object including
  #     [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html).
  # `pat`
  # :   Pattern for argument, defaults to `t` if it responds to match.
  #
  #
  # ```ruby
  # accept(t, pat, &block)
  # ```
  sig {params(args: T.untyped, blk: T.untyped).returns(T.untyped)}
  def accept(*args, &blk); end

  # See
  # [`accept`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-accept).
  sig {params(args: T.untyped, blk: T.untyped).returns(T.untyped)}
  def self.accept(*args, &blk); end

  # Directs to reject specified class argument.
  #
  # `t`
  # :   Argument class specifier, any object including
  #     [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html).
  #
  #
  # ```ruby
  # reject(t)
  # ```
  sig {params(args: T.untyped, blk: T.untyped).returns(T.untyped)}
  def reject(*args, &blk); end

  # See
  # [`reject`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-reject).
  sig {params(args: T.untyped, blk: T.untyped).returns(T.untyped)}
  def self.reject(*args, &blk); end

  # Heading banner preceding summary.
  sig {params(value: T.untyped).returns(T.untyped)}
  def banner=(value); end

  # Heading banner preceding summary.
  sig {params(value: T.untyped).returns(T.untyped)}
  def set_banner(value); end

  # Program name to be emitted in error message and default banner, defaults to
  # $0.
  sig {params(value: T.untyped).returns(T.untyped)}
  def program_name=(value); end

  # Program name to be emitted in error message and default banner, defaults to
  # $0.
  sig {params(value: T.untyped).returns(T.untyped)}
  def set_program_name(value); end

  # Width for option list portion of summary. Must be
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html).
  sig {returns(T.untyped)}
  def summary_width(); end

  # Width for option list portion of summary. Must be
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html).
  sig {params(value: T.untyped).returns(T.untyped)}
  def summary_width=(value); end

  # Width for option list portion of summary. Must be
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html).
  sig {params(value: T.untyped).returns(T.untyped)}
  def set_summary_width(value); end

  # Indentation for summary. Must be
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) (or have +
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) method).
  sig {returns(T.untyped)}
  def summary_indent(); end

  # Indentation for summary. Must be
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) (or have +
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) method).
  sig {params(value: T.untyped).returns(T.untyped)}
  def summary_indent=(value); end

  # Indentation for summary. Must be
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) (or have +
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) method).
  sig {params(value: T.untyped).returns(T.untyped)}
  def set_summary_indent(value); end

  # Strings to be parsed in default.
  sig {returns(T.untyped)}
  def default_argv(); end

  # Strings to be parsed in default.
  sig {params(value: T.untyped).returns(T.untyped)}
  def default_argv=(value); end

  # Heading banner preceding summary.
  sig {returns(T.untyped)}
  def banner(); end

  # Program name to be emitted in error message and default banner, defaults to
  # $0.
  sig {returns(T.untyped)}
  def program_name(); end

  # Version
  sig {params(value: T.untyped).returns(T.untyped)}
  def version=(value); end

  # Release code
  sig {params(value: T.untyped).returns(T.untyped)}
  def release=(value); end

  # Version
  sig {returns(T.untyped)}
  def version(); end

  # Release code
  sig {returns(T.untyped)}
  def release(); end

  # Returns version string from
  # [`program_name`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#attribute-i-program_name),
  # version and release.
  sig {returns(T.untyped)}
  def ver(); end

  sig {params(mesg: T.untyped).returns(T.untyped)}
  def warn(mesg = $!); end

  sig {params(mesg: T.untyped).returns(T.untyped)}
  def abort(mesg = $!); end

  # Subject of
  # [`on`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-on) /
  # [`on_head`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-on_head),
  # [`accept`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-accept)
  # /
  # [`reject`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-reject)
  sig {returns(T.untyped)}
  def top(); end

  # Subject of
  # [`on_tail`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-on_tail).
  sig {returns(T.untyped)}
  def base(); end

  # Pushes a new List.
  sig {returns(T.untyped)}
  def new(); end

  # Removes the last List.
  sig {returns(T.untyped)}
  def remove(); end

  # Puts option summary into `to` and returns `to`. Yields each line if a block
  # is given.
  #
  # `to`
  # :   Output destination, which must have method <<. Defaults to [].
  # `width`
  # :   Width of left side, defaults to @summary\_width.
  # `max`
  # :   Maximum length allowed for left side, defaults to `width` - 1.
  # `indent`
  # :   Indentation, defaults to @summary\_indent.
  sig do
    params(
      to: T.untyped,
      width: T.untyped,
      max: T.untyped,
      indent: T.untyped,
      blk: T.untyped
    ).returns(T.untyped)
  end
  def summarize(to = [], width = @summary_width, max = width - 1, indent = @summary_indent, &blk); end

  # Returns option summary string.
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-to_s)
  sig {returns(T.untyped)}
  def help(); end

  # Returns option summary list.
  sig {returns(T.untyped)}
  def to_a(); end

  # Checks if an argument is given twice, in which case an
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html) is
  # raised. Called from OptionParser#switch only.
  #
  # `obj`
  # :   New argument.
  # `prv`
  # :   Previously specified argument.
  # `msg`
  # :   [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html)
  #     message.
  sig {params(obj: T.untyped, prv: T.untyped, msg: T.untyped).returns(T.untyped)}
  def notwice(obj, prv, msg); end

  # Creates an
  # [`OptionParser::Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
  # from the parameters. The parsed argument value is passed to the given block,
  # where it can be processed.
  #
  # See at the beginning of
  # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html) for
  # some full examples.
  #
  # `opts` can include the following elements:
  #
  # Argument style:
  # :   One of the following:
  #
  # ```
  # :NONE, :REQUIRED, :OPTIONAL
  # ```
  #
  # Argument pattern:
  # :   Acceptable option argument format, must be pre-defined with
  #     [`OptionParser.accept`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-c-accept)
  #     or
  #     [`OptionParser#accept`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-accept),
  #     or [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html). This can
  #     appear once or assigned as
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) if not
  #     present, otherwise causes an
  #     [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html).
  #     Examples:
  #
  # ```
  # Float, Time, Array
  # ```
  #
  # Possible argument values:
  # :   [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) or
  #     [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  #
  # ```ruby
  # [:text, :binary, :auto]
  # %w[iso-2022-jp shift_jis euc-jp utf8 binary]
  # { "jis" => "iso-2022-jp", "sjis" => "shift_jis" }
  # ```
  #
  # Long style switch:
  # :   Specifies a long style switch which takes a mandatory, optional or no
  #     argument. It's a string of the following form:
  #
  # ```ruby
  # "--switch=MANDATORY" or "--switch MANDATORY"
  # "--switch[=OPTIONAL]"
  # "--switch"
  # ```
  #
  # Short style switch:
  # :   Specifies short style switch which takes a mandatory, optional or no
  #     argument. It's a string of the following form:
  #
  # ```ruby
  # "-xMANDATORY"
  # "-x[OPTIONAL]"
  # "-x"
  # ```
  #
  #     There is also a special form which matches character range (not full set
  #     of regular expression):
  #
  # ```ruby
  # "-[a-z]MANDATORY"
  # "-[a-z][OPTIONAL]"
  # "-[a-z]"
  # ```
  #
  # Argument style and description:
  # :   Instead of specifying mandatory or optional arguments directly in the
  #     switch parameter, this separate parameter can be used.
  #
  # ```ruby
  # "=MANDATORY"
  # "=[OPTIONAL]"
  # ```
  #
  # Description:
  # :   Description string for the option.
  #
  # ```ruby
  # "Run verbosely"
  # ```
  #
  #     If you give multiple description strings, each string will be printed
  #     line by line.
  #
  # Handler:
  # :   Handler for the parsed argument value. Either give a block or pass a
  #     [`Proc`](https://docs.ruby-lang.org/en/2.7.0/Proc.html) or
  #     [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) as an
  #     argument.
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def make_switch(opts, block = nil); end

  # Also aliased as:
  # [`def_option`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-def_option)
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def define(*opts, &block); end

  # Alias for:
  # [`define`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-define)
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def def_option(*opts, &block); end

  # Add option switch and handler. See
  # [`make_switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-make_switch)
  # for an explanation of parameters.
  sig do
    params(
      type: T.any(T.class_of(TrueClass), T.class_of(FalseClass)),
      opts: T.untyped,
      block: T.nilable(T.proc.params(arg0: T::Boolean).void)
    )
      .returns(T.untyped)
  end
  sig do
    params(
      type: T.any(T.class_of(Shellwords), T.class_of(Array)),
      opts: T.untyped,
      block: T.nilable(T.proc.params(arg0: T::Array[String]).void)
    )
      .returns(T.untyped)
  end
  sig do
    type_parameters(:Type)
      .params(
        type: T::Class[T.type_parameter(:Type)],
        opts: T.untyped,
        block: T.nilable(T.proc.params(arg0: T.type_parameter(:Type)).void)
      )
      .returns(T.untyped)
  end
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def on(type=T.unsafe(nil), *opts, &block); end

  # Also aliased as:
  # [`def_head_option`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-def_head_option)
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def define_head(*opts, &block); end

  # Alias for:
  # [`define_head`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-define_head)
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def def_head_option(*opts, &block); end

  # Add option switch like with
  # [`on`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-on),
  # but at head of summary.
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def on_head(*opts, &block); end

  # Also aliased as:
  # [`def_tail_option`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-def_tail_option)
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def define_tail(*opts, &block); end

  # Alias for:
  # [`define_tail`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-define_tail)
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def def_tail_option(*opts, &block); end

  # Add option switch like with
  # [`on`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-on),
  # but at tail of summary.
  sig {params(opts: T.untyped, block: T.untyped).returns(T.untyped)}
  def on_tail(*opts, &block); end

  # Add separator in summary.
  sig {params(string: T.untyped).returns(T.untyped)}
  def separator(string); end

  # Parses command line arguments `argv` in order. When a block is given, each
  # non-option argument is yielded. When optional `into` keyword argument is
  # provided, the parsed option values are stored there via `[]=` method (so it
  # can be [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html), or
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html), or
  # other similar object).
  #
  # Returns the rest of `argv` left unparsed.
  sig {params(argv: T.untyped, into: T.untyped, nonopt: T.untyped).returns(T.untyped)}
  def order(*argv, into: nil, &nonopt); end

  # Same as
  # [`order`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-order),
  # but removes switches destructively. Non-option arguments remain in `argv`.
  sig {params(argv: T.untyped, into: T.untyped, nonopt: T.untyped).returns(T.untyped)}
  def order!(argv = default_argv, into: nil, &nonopt); end

  sig {params(argv: T.untyped, setter: T.untyped, nonopt: T.untyped).returns(T.untyped)}
  def parse_in_order(argv = default_argv, setter = nil, &nonopt); end

  # Parses command line arguments `argv` in permutation mode and returns list of
  # non-option arguments. When optional `into` keyword argument is provided, the
  # parsed option values are stored there via `[]=` method (so it can be
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html), or
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html), or
  # other similar object).
  sig {params(argv: T.untyped, into: T.untyped).returns(T.untyped)}
  def permute(*argv, into: nil); end

  # Same as
  # [`permute`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-permute),
  # but removes switches destructively. Non-option arguments remain in `argv`.
  sig {params(argv: T.untyped, into: T.untyped).returns(T.untyped)}
  def permute!(argv = default_argv, into: nil); end

  # Parses command line arguments `argv` in order when environment variable
  # POSIXLY\_CORRECT is set, and in permutation mode otherwise. When optional
  # `into` keyword argument is provided, the parsed option values are stored
  # there via `[]=` method (so it can be
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html), or
  # [`OpenStruct`](https://docs.ruby-lang.org/en/2.7.0/OpenStruct.html), or
  # other similar object).
  sig {params(argv: T.untyped, into: T.untyped).returns(T.untyped)}
  def parse(*argv, into: nil); end

  # Same as
  # [`parse`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-parse),
  # but removes switches destructively. Non-option arguments remain in `argv`.
  sig {params(argv: T.untyped, into: T.untyped).returns(T.untyped)}
  def parse!(argv = default_argv, into: nil); end

  # Wrapper method for getopts.rb.
  #
  # ```ruby
  # params = ARGV.getopts("ab:", "foo", "bar:", "zot:Z;zot option")
  # # params["a"] = true   # -a
  # # params["b"] = "1"    # -b1
  # # params["foo"] = "1"  # --foo
  # # params["bar"] = "x"  # --bar x
  # # params["zot"] = "z"  # --zot Z
  # ```
  sig {params(args: T.untyped).returns(T.untyped)}
  def getopts(*args); end

  # See
  # [`getopts`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-getopts).
  sig {params(args: T.untyped).returns(T.untyped)}
  def self.getopts(*args); end

  # Traverses @stack, sending each element method `id` with `args` and `block`.
  sig {params(id: T.untyped, args: T.untyped, block: T.untyped).returns(T.untyped)}
  def visit(id, *args, &block); end

  # Searches `key` in @stack for `id` hash and returns or yields the result.
  sig {params(id: T.untyped, key: T.untyped).returns(T.untyped)}
  def search(id, key); end

  # Completes shortened long style option switch and returns pair of canonical
  # switch and switch descriptor
  # [`OptionParser::Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html).
  #
  # `typ`
  # :   Searching table.
  # `opt`
  # :   Searching key.
  # `icase`
  # :   Search case insensitive if true.
  # `pat`
  # :   Optional pattern for completion.
  sig do
    params(
      typ: T.untyped,
      opt: T.untyped,
      icase: T.untyped,
      pat: T.untyped
    ).returns(T.untyped)
  end
  def complete(typ, opt, icase = false, *pat); end

  sig {params(word: T.untyped).returns(T.untyped)}
  def candidate(word); end

  # Loads options from file names as `filename`. Does nothing when the file is
  # not present. Returns whether successfully loaded.
  #
  # `filename` defaults to basename of the program without suffix in a directory
  # ~/.options, then the basename with '.options' suffix under XDG and Haiku
  # standard places.
  sig {params(filename: T.untyped).returns(T.untyped)}
  def load(filename = nil); end

  # Parses environment variable `env` or its uppercase with splitting like a
  # shell.
  #
  # `env` defaults to the basename of the program.
  sig {params(env: T.untyped).returns(T.untyped)}
  def environment(env = File.basename($0, '.*')); end

  # Keyword completion module. This allows partial arguments to be specified and
  # resolved against a list of acceptable values.
  module Completion
    sig {params(key: T.untyped, icase: T.untyped).returns(T.untyped)}
    def self.regexp(key, icase); end

    sig do
      params(
        key: T.untyped,
        icase: T.untyped,
        pat: T.untyped,
        block: T.untyped
      ).returns(T.untyped)
    end
    def self.candidate(key, icase = false, pat = nil, &block); end

    sig {params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped)}
    def candidate(key, icase = false, pat = nil); end

    sig {params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped)}
    def complete(key, icase = false, pat = nil); end

    sig {params(opt: T.untyped, val: T.untyped, _: T.untyped).returns(T.untyped)}
    def convert(opt = nil, val = nil, *_); end
  end

  # Map from option/keyword string to object with completion.
  class OptionMap < Hash
    include OptionParser::Completion

    K = type_member {{fixed: T.untyped}}
    V = type_member {{fixed: T.untyped}}
    Elem = type_member {{fixed: T.untyped}}

    sig {params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped)}
    def candidate(key, icase = false, pat = nil); end

    sig {params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped)}
    def complete(key, icase = false, pat = nil); end

    sig {params(opt: T.untyped, val: T.untyped).returns(T.untyped)}
    def convert(opt = nil, val = nil); end
  end

  # Individual switch class. Not important to the user.
  #
  # Defined within
  # [`Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html) are
  # several Switch-derived classes: NoArgument, RequiredArgument, etc.
  class Switch
    sig {returns(T.untyped)}
    def pattern(); end

    sig {returns(T.untyped)}
    def conv(); end

    sig {returns(T.untyped)}
    def short(); end

    sig {returns(T.untyped)}
    def long(); end

    sig {returns(T.untyped)}
    def arg(); end

    sig {returns(T.untyped)}
    def desc(); end

    sig {returns(T.untyped)}
    def block(); end

    # Guesses argument style from `arg`. Returns corresponding
    # [`OptionParser::Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
    # class (OptionalArgument, etc.).
    sig {params(arg: T.untyped).returns(T.untyped)}
    def self.guess(arg); end

    sig {params(arg: T.untyped, t: T.untyped).returns(T.untyped)}
    def self.incompatible_argument_styles(arg, t); end

    sig {returns(T.untyped)}
    def self.pattern(); end

    sig do
      params(
        pattern: T.untyped,
        conv: T.untyped,
        short: T.untyped,
        long: T.untyped,
        arg: T.untyped,
        desc: T.untyped,
        block: T.untyped,
        _block: T.untyped
      ).void
    end
    def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

    # Parses `arg` and returns rest of `arg` and matched portion to the argument
    # pattern. Yields when the pattern doesn't match substring.
    sig {params(arg: T.untyped).returns(T.untyped)}
    def parse_arg(arg); end

    # Parses argument, converts and returns `arg`, `block` and result of
    # conversion. Yields at semi-error condition instead of raising an
    # exception.
    sig {params(arg: T.untyped, val: T.untyped).returns(T.untyped)}
    def conv_arg(arg, val = []); end

    # Produces the summary text. Each line of the summary is yielded to the
    # block (without newline).
    #
    # `sdone`
    # :   Already summarized short style options keyed hash.
    # `ldone`
    # :   Already summarized long style options keyed hash.
    # `width`
    # :   Width of left side (option part). In other words, the right side
    #     (description part) starts after `width` columns.
    # `max`
    # :   Maximum width of left side -> the options are filled within `max`
    #     columns.
    # `indent`
    # :   Prefix string indents all summarized lines.
    sig do
      params(
        sdone: T.untyped,
        ldone: T.untyped,
        width: T.untyped,
        max: T.untyped,
        indent: T.untyped
      ).returns(T.untyped)
    end
    def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

    sig {params(to: T.untyped).returns(T.untyped)}
    def add_banner(to); end

    sig {params(str: T.untyped).returns(T::Boolean)}
    def match_nonswitch?(str); end

    # Main name of the switch.
    sig {returns(T.untyped)}
    def switch_name(); end

    sig {params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped)}
    def compsys(sdone, ldone); end

    # [`Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
    # that takes no arguments.
    class NoArgument < OptionParser::Switch
      # Raises an exception if any arguments given.
      sig {params(arg: T.untyped, argv: T.untyped).returns(T.untyped)}
      def parse(arg, argv); end

      sig {params(_: T.untyped).returns(T.untyped)}
      def self.incompatible_argument_styles(*_); end

      sig {returns(T.untyped)}
      def self.pattern(); end

      sig {returns(T.untyped)}
      def pattern(); end

      sig {returns(T.untyped)}
      def conv(); end

      sig {returns(T.untyped)}
      def short(); end

      sig {returns(T.untyped)}
      def long(); end

      sig {returns(T.untyped)}
      def arg(); end

      sig {returns(T.untyped)}
      def desc(); end

      sig {returns(T.untyped)}
      def block(); end

      sig {params(arg: T.untyped).returns(T.untyped)}
      def self.guess(arg); end

      sig do
        params(
          pattern: T.untyped,
          conv: T.untyped,
          short: T.untyped,
          long: T.untyped,
          arg: T.untyped,
          desc: T.untyped,
          block: T.untyped,
          _block: T.untyped
        ).void
      end
      def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

      sig {params(arg: T.untyped).returns(T.untyped)}
      def parse_arg(arg); end

      sig {params(arg: T.untyped, val: T.untyped).returns(T.untyped)}
      def conv_arg(arg, val = []); end

      sig do
        params(
          sdone: T.untyped,
          ldone: T.untyped,
          width: T.untyped,
          max: T.untyped,
          indent: T.untyped
        ).returns(T.untyped)
      end
      def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

      sig {params(to: T.untyped).returns(T.untyped)}
      def add_banner(to); end

      sig {params(str: T.untyped).returns(T::Boolean)}
      def match_nonswitch?(str); end

      sig {returns(T.untyped)}
      def switch_name(); end

      sig {params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped)}
      def compsys(sdone, ldone); end
    end

    # [`Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
    # that takes an argument.
    class RequiredArgument < OptionParser::Switch
      # Raises an exception if argument is not present.
      sig {params(arg: T.untyped, argv: T.untyped).returns(T.untyped)}
      def parse(arg, argv); end

      sig {returns(T.untyped)}
      def pattern(); end

      sig {returns(T.untyped)}
      def conv(); end

      sig {returns(T.untyped)}
      def short(); end

      sig {returns(T.untyped)}
      def long(); end

      sig {returns(T.untyped)}
      def arg(); end

      sig {returns(T.untyped)}
      def desc(); end

      sig {returns(T.untyped)}
      def block(); end

      sig {params(arg: T.untyped).returns(T.untyped)}
      def self.guess(arg); end

      sig {params(arg: T.untyped, t: T.untyped).returns(T.untyped)}
      def self.incompatible_argument_styles(arg, t); end

      sig {returns(T.untyped)}
      def self.pattern(); end

      sig do
        params(
          pattern: T.untyped,
          conv: T.untyped,
          short: T.untyped,
          long: T.untyped,
          arg: T.untyped,
          desc: T.untyped,
          block: T.untyped,
          _block: T.untyped
        ).void
      end
      def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

      sig {params(arg: T.untyped).returns(T.untyped)}
      def parse_arg(arg); end

      sig {params(arg: T.untyped, val: T.untyped).returns(T.untyped)}
      def conv_arg(arg, val = []); end

      sig do
        params(
          sdone: T.untyped,
          ldone: T.untyped,
          width: T.untyped,
          max: T.untyped,
          indent: T.untyped
        ).returns(T.untyped)
      end
      def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

      sig {params(to: T.untyped).returns(T.untyped)}
      def add_banner(to); end

      sig {params(str: T.untyped).returns(T::Boolean)}
      def match_nonswitch?(str); end

      sig {returns(T.untyped)}
      def switch_name(); end

      sig {params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped)}
      def compsys(sdone, ldone); end
    end

    # [`Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
    # that can omit argument.
    class OptionalArgument < OptionParser::Switch
      # Parses argument if given, or uses default value.
      sig {params(arg: T.untyped, argv: T.untyped, error: T.untyped).returns(T.untyped)}
      def parse(arg, argv, &error); end

      sig {returns(T.untyped)}
      def pattern(); end

      sig {returns(T.untyped)}
      def conv(); end

      sig {returns(T.untyped)}
      def short(); end

      sig {returns(T.untyped)}
      def long(); end

      sig {returns(T.untyped)}
      def arg(); end

      sig {returns(T.untyped)}
      def desc(); end

      sig {returns(T.untyped)}
      def block(); end

      sig {params(arg: T.untyped).returns(T.untyped)}
      def self.guess(arg); end

      sig {params(arg: T.untyped, t: T.untyped).returns(T.untyped)}
      def self.incompatible_argument_styles(arg, t); end

      sig {returns(T.untyped)}
      def self.pattern(); end

      sig do
        params(
          pattern: T.untyped,
          conv: T.untyped,
          short: T.untyped,
          long: T.untyped,
          arg: T.untyped,
          desc: T.untyped,
          block: T.untyped,
          _block: T.untyped
        ).void
      end
      def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

      sig {params(arg: T.untyped).returns(T.untyped)}
      def parse_arg(arg); end

      sig {params(arg: T.untyped, val: T.untyped).returns(T.untyped)}
      def conv_arg(arg, val = []); end

      sig do
        params(
          sdone: T.untyped,
          ldone: T.untyped,
          width: T.untyped,
          max: T.untyped,
          indent: T.untyped
        ).returns(T.untyped)
      end
      def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

      sig {params(to: T.untyped).returns(T.untyped)}
      def add_banner(to); end

      sig {params(str: T.untyped).returns(T::Boolean)}
      def match_nonswitch?(str); end

      sig {returns(T.untyped)}
      def switch_name(); end

      sig {params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped)}
      def compsys(sdone, ldone); end
    end

    # [`Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
    # that takes an argument, which does not begin with '-'.
    class PlacedArgument < OptionParser::Switch
      # Returns nil if argument is not present or begins with '-'.
      sig {params(arg: T.untyped, argv: T.untyped, error: T.untyped).returns(T.untyped)}
      def parse(arg, argv, &error); end

      sig {returns(T.untyped)}
      def pattern(); end

      sig {returns(T.untyped)}
      def conv(); end

      sig {returns(T.untyped)}
      def short(); end

      sig {returns(T.untyped)}
      def long(); end

      sig {returns(T.untyped)}
      def arg(); end

      sig {returns(T.untyped)}
      def desc(); end

      sig {returns(T.untyped)}
      def block(); end

      sig {params(arg: T.untyped).returns(T.untyped)}
      def self.guess(arg); end

      sig {params(arg: T.untyped, t: T.untyped).returns(T.untyped)}
      def self.incompatible_argument_styles(arg, t); end

      sig {returns(T.untyped)}
      def self.pattern(); end

      sig do
        params(
          pattern: T.untyped,
          conv: T.untyped,
          short: T.untyped,
          long: T.untyped,
          arg: T.untyped,
          desc: T.untyped,
          block: T.untyped,
          _block: T.untyped
        ).void
      end
      def initialize(pattern = nil, conv = nil, short = nil, long = nil, arg = nil, desc = ([] if short or long), block = nil, &_block); end

      sig {params(arg: T.untyped).returns(T.untyped)}
      def parse_arg(arg); end

      sig {params(arg: T.untyped, val: T.untyped).returns(T.untyped)}
      def conv_arg(arg, val = []); end

      sig do
        params(
          sdone: T.untyped,
          ldone: T.untyped,
          width: T.untyped,
          max: T.untyped,
          indent: T.untyped
        ).returns(T.untyped)
      end
      def summarize(sdone = [], ldone = [], width = 1, max = width - 1, indent = ""); end

      sig {params(to: T.untyped).returns(T.untyped)}
      def add_banner(to); end

      sig {params(str: T.untyped).returns(T::Boolean)}
      def match_nonswitch?(str); end

      sig {returns(T.untyped)}
      def switch_name(); end

      sig {params(sdone: T.untyped, ldone: T.untyped).returns(T.untyped)}
      def compsys(sdone, ldone); end
    end
  end

  # Simple option list providing mapping from short and/or long option string to
  # [`OptionParser::Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
  # and mapping from acceptable argument to matching pattern and converter pair.
  # Also provides summary feature.
  class List
    # Map from acceptable argument types to pattern and converter pairs.
    sig {returns(T.untyped)}
    def atype(); end

    # Map from short style option switches to actual switch objects.
    sig {returns(T.untyped)}
    def short(); end

    # Map from long style option switches to actual switch objects.
    sig {returns(T.untyped)}
    def long(); end

    # [`List`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/List.html) of
    # all switches and summary string.
    sig {returns(T.untyped)}
    def list(); end

    sig {void}
    def initialize(); end

    # See
    # [`OptionParser.accept`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-c-accept).
    sig {params(t: T.untyped, pat: T.untyped, block: T.untyped).returns(T.untyped)}
    def accept(t, pat = /.*/m, &block); end

    # See
    # [`OptionParser.reject`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-c-reject).
    sig {params(t: T.untyped).returns(T.untyped)}
    def reject(t); end

    # Adds `sw` according to `sopts`, `lopts` and `nlopts`.
    #
    # `sw`
    # :   [`OptionParser::Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
    #     instance to be added.
    # `sopts`
    # :   Short style option list.
    # `lopts`
    # :   Long style option list.
    # `nlopts`
    # :   Negated long style options list.
    sig do
      params(
        sw: T.untyped,
        sopts: T.untyped,
        lopts: T.untyped,
        nsw: T.untyped,
        nlopts: T.untyped
      ).returns(T.untyped)
    end
    def update(sw, sopts, lopts, nsw = nil, nlopts = nil); end

    # Inserts `switch` at the head of the list, and associates short, long and
    # negated long options. Arguments are:
    #
    # `switch`
    # :   [`OptionParser::Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
    #     instance to be inserted.
    # `short_opts`
    # :   [`List`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/List.html)
    #     of short style options.
    # `long_opts`
    # :   [`List`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/List.html)
    #     of long style options.
    # `nolong_opts`
    # :   [`List`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/List.html)
    #     of long style options with "no-" prefix.
    #
    #
    # ```ruby
    # prepend(switch, short_opts, long_opts, nolong_opts)
    # ```
    sig {params(args: T.untyped).returns(T.untyped)}
    def prepend(*args); end

    # Appends `switch` at the tail of the list, and associates short, long and
    # negated long options. Arguments are:
    #
    # `switch`
    # :   [`OptionParser::Switch`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Switch.html)
    #     instance to be inserted.
    # `short_opts`
    # :   [`List`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/List.html)
    #     of short style options.
    # `long_opts`
    # :   [`List`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/List.html)
    #     of long style options.
    # `nolong_opts`
    # :   [`List`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/List.html)
    #     of long style options with "no-" prefix.
    #
    #
    # ```ruby
    # append(switch, short_opts, long_opts, nolong_opts)
    # ```
    sig {params(args: T.untyped).returns(T.untyped)}
    def append(*args); end

    # Searches `key` in `id` list. The result is returned or yielded if a block
    # is given. If it isn't found, nil is returned.
    sig {params(id: T.untyped, key: T.untyped).returns(T.untyped)}
    def search(id, key); end

    # Searches list `id` for `opt` and the optional patterns for completion
    # `pat`. If `icase` is true, the search is case insensitive. The result is
    # returned or yielded if a block is given. If it isn't found, nil is
    # returned.
    sig do
      params(
        id: T.untyped,
        opt: T.untyped,
        icase: T.untyped,
        pat: T.untyped,
        block: T.untyped
      ).returns(T.untyped)
    end
    def complete(id, opt, icase = false, *pat, &block); end

    # Iterates over each option, passing the option to the `block`.
    sig {params(block: T.untyped).returns(T.untyped)}
    def each_option(&block); end

    # Creates the summary table, passing each line to the `block` (without
    # newline). The arguments `args` are passed along to the summarize method
    # which is called on every option.
    sig {params(args: T.untyped, block: T.untyped).returns(T.untyped)}
    def summarize(*args, &block); end

    sig {params(to: T.untyped).returns(T.untyped)}
    def add_banner(to); end

    sig {params(args: T.untyped, block: T.untyped).returns(T.untyped)}
    def compsys(*args, &block); end
  end

  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) with completion
  # search feature. See
  # [`OptionParser::Completion`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Completion.html).
  class CompletingHash < Hash
    include OptionParser::Completion
    K = type_member {{fixed: T.untyped}}
    V = type_member {{fixed: T.untyped}}
    Elem = type_member {{fixed: T.untyped}}

    # Completion for hash key.
    sig {params(key: T.untyped).returns(T.untyped)}
    def match(key); end

    sig {params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped)}
    def candidate(key, icase = false, pat = nil); end

    sig {params(key: T.untyped, icase: T.untyped, pat: T.untyped).returns(T.untyped)}
    def complete(key, icase = false, pat = nil); end

    sig {params(opt: T.untyped, val: T.untyped).returns(T.untyped)}
    def convert(opt = nil, val = nil); end
  end

  # Base class of exceptions from
  # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html).
  class ParseError < RuntimeError
    # [`Reason`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/ParseError.html#Reason)
    # which caused the error.
    Reason = T.let(nil, T.untyped)

    sig {params(args: T.untyped, additional: T.untyped).void}
    def initialize(*args, additional: nil); end

    sig {returns(T.untyped)}
    def args(); end

    sig {params(value: T.untyped).returns(T.untyped)}
    def reason=(value); end

    # Pushes back erred argument(s) to `argv`.
    sig {params(argv: T.untyped).returns(T.untyped)}
    def recover(argv); end

    sig {params(array: T.untyped).returns(T.untyped)}
    def self.filter_backtrace(array); end

    sig {params(array: T.untyped).returns(T.untyped)}
    def set_backtrace(array); end

    sig {params(opt: T.untyped, eq: T.untyped).returns(T.untyped)}
    def set_option(opt, eq); end

    sig {returns(T.untyped)}
    def reason(); end

    sig {returns(T.untyped)}
    def inspect(); end

    # Default stringizing method to emit standard error message.
    #
    # Also aliased as:
    # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/ParseError.html#method-i-to_s)
    sig {returns(T.untyped)}
    def message(); end
  end

  # Raises when ambiguously completable string is encountered.
  class AmbiguousOption < OptionParser::ParseError
    Reason = T.let(nil, T.untyped)
  end

  # Raises when there is an argument for a switch which takes no argument.
  class NeedlessArgument < OptionParser::ParseError
    Reason = T.let(nil, T.untyped)
  end

  # Raises when a switch with mandatory argument has no argument.
  class MissingArgument < OptionParser::ParseError
    Reason = T.let(nil, T.untyped)
  end

  # Raises when switch is undefined.
  class InvalidOption < OptionParser::ParseError
    Reason = T.let(nil, T.untyped)
  end

  # Raises when the given argument does not match required format.
  class InvalidArgument < OptionParser::ParseError
    Reason = T.let(nil, T.untyped)
  end

  # Raises when the given argument word can't be completed uniquely.
  class AmbiguousArgument < OptionParser::InvalidArgument
    Reason = T.let(nil, T.untyped)
  end

  # Extends command line arguments array (ARGV) to parse itself.
  module Arguable
    # Sets
    # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
    # object, when `opt` is `false` or `nil`, methods
    # [`OptionParser::Arguable#options`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Arguable.html#method-i-options)
    # and
    # [`OptionParser::Arguable#options=`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/Arguable.html#method-i-options-3D)
    # are undefined. Thus, there is no ways to access the
    # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
    # object via the receiver object.
    sig {params(opt: T.untyped).returns(T.untyped)}
    def options=(opt); end

    # Actual
    # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
    # object, automatically created if nonexistent.
    #
    # If called with a block, yields the
    # [`OptionParser`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html)
    # object and returns the result of the block. If an
    # [`OptionParser::ParseError`](https://docs.ruby-lang.org/en/2.7.0/OptionParser/ParseError.html)
    # exception occurs in the block, it is rescued, a error message printed to
    # STDERR and `nil` returned.
    sig {returns(T.untyped)}
    def options(); end

    # Parses `self` destructively in order and returns `self` containing the
    # rest arguments left unparsed.
    sig {params(blk: T.untyped).returns(T.untyped)}
    def order!(&blk); end

    # Parses `self` destructively in permutation mode and returns `self`
    # containing the rest arguments left unparsed.
    sig {returns(T.untyped)}
    def permute!(); end

    # Parses `self` destructively and returns `self` containing the rest
    # arguments left unparsed.
    sig {returns(T.untyped)}
    def parse!(); end

    # Substitution of getopts is possible as follows. Also see
    # [`OptionParser#getopts`](https://docs.ruby-lang.org/en/2.7.0/OptionParser.html#method-i-getopts).
    #
    # ```ruby
    # def getopts(*args)
    #   ($OPT = ARGV.getopts(*args)).each do |opt, val|
    #     eval "$OPT_#{opt.gsub(/[^A-Za-z0-9_]/, '_')} = val"
    #   end
    # rescue OptionParser::ParseError
    # end
    # ```
    sig {params(args: T.untyped).returns(T.untyped)}
    def getopts(*args); end

    # Initializes instance variable.
    sig {params(obj: T.untyped).returns(T.untyped)}
    def self.extend_object(obj); end

    sig {params(args: T.untyped).void}
    def initialize(*args); end
  end

  # Acceptable argument classes. Now contains DecimalInteger, OctalInteger and
  # DecimalNumeric. See Acceptable argument classes (in source code).
  module Acceptables
    DecimalInteger = T.let(nil, T.untyped)
    DecimalNumeric = T.let(nil, T.untyped)
    OctalInteger = T.let(nil, T.untyped)
  end
end

OptParse = OptionParser
