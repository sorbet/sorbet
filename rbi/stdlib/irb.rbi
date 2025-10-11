# typed: __STDLIB_INTERNAL

# [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) stands for "interactive
# Ruby" and is a tool to interactively execute Ruby expressions read from the
# standard input.
#
# The `irb` command from your shell will start the interpreter.
#
# ## Usage
#
# Use of irb is easy if you know Ruby.
#
# When executing irb, prompts are displayed as follows. Then, enter the Ruby
# expression. An input is executed when it is syntactically complete.
#
# ```
# $ irb
# irb(main):001:0> 1+2
# #=> 3
# irb(main):002:0> class Foo
# irb(main):003:1>  def foo
# irb(main):004:2>    print 1
# irb(main):005:2>  end
# irb(main):006:1> end
# #=> nil
# ```
#
# The singleline editor module or multiline editor module can be used with irb.
# Use of multiline editor is default if it's installed.
#
# ## Command line options
#
# ```
# Usage:  irb.rb [options] [programfile] [arguments]
#   -f                Suppress read of ~/.irbrc
#   -d                Set $DEBUG to true (same as `ruby -d')
#   -r load-module    Same as `ruby -r'
#   -I path           Specify $LOAD_PATH directory
#   -U                Same as `ruby -U`
#   -E enc            Same as `ruby -E`
#   -w                Same as `ruby -w`
#   -W[level=2]       Same as `ruby -W`
#   --inspect         Use `inspect' for output (default except for bc mode)
#   --noinspect       Don't use inspect for output
#   --multiline       Use multiline editor module
#   --nomultiline     Don't use multiline editor module
#   --singleline      Use singleline editor module
#   --nosingleline    Don't use singleline editor module
#   --colorize        Use colorization
#   --nocolorize      Don't use colorization
#   --prompt prompt-mode
#   --prompt-mode prompt-mode
#                     Switch prompt mode. Pre-defined prompt modes are
#                     `default', `simple', `xmp' and `inf-ruby'
#   --inf-ruby-mode   Use prompt appropriate for inf-ruby-mode on emacs.
#                     Suppresses --multiline and --singleline.
#   --simple-prompt   Simple prompt mode
#   --noprompt        No prompt mode
#   --tracer          Display trace for each execution of commands.
#   --back-trace-limit n
#                     Display backtrace top n and tail n. The default
#                     value is 16.
#   -v, --version     Print the version of irb
# ```
#
# ## Configuration
#
# [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) reads from `~/.irbrc`
# when it's invoked.
#
# If `~/.irbrc` doesn't exist, `irb` will try to read in the following order:
#
# *   `.irbrc`
# *   `irb.rc`
# *   `_irbrc`
# *   `$irbrc`
#
#
# The following are alternatives to the command line options. To use them type
# as follows in an `irb` session:
#
# ```
# IRB.conf[:IRB_NAME]="irb"
# IRB.conf[:INSPECT_MODE]=nil
# IRB.conf[:IRB_RC] = nil
# IRB.conf[:BACK_TRACE_LIMIT]=16
# IRB.conf[:USE_LOADER] = false
# IRB.conf[:USE_MULTILINE] = nil
# IRB.conf[:USE_SINGLELINE] = nil
# IRB.conf[:USE_COLORIZE] = true
# IRB.conf[:USE_TRACER] = false
# IRB.conf[:IGNORE_SIGINT] = true
# IRB.conf[:IGNORE_EOF] = false
# IRB.conf[:PROMPT_MODE] = :DEFAULT
# IRB.conf[:PROMPT] = {...}
# ```
#
# ### Auto indentation
#
# To disable auto-indent mode in irb, add the following to your `.irbrc`:
#
# ```ruby
# IRB.conf[:AUTO_INDENT] = false
# ```
#
# ### Autocompletion
#
# To enable autocompletion for irb, add the following to your `.irbrc`:
#
# ```ruby
# require 'irb/completion'
# ```
#
# ### History
#
# By default, irb will store the last 1000 commands you used in
# `IRB.conf[:HISTORY_FILE]` (`~/.irb_history` by default).
#
# If you want to disable history, add the following to your `.irbrc`:
#
# ```ruby
# IRB.conf[:SAVE_HISTORY] = nil
# ```
#
# See
# [`IRB::Context#save_history=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-save_history-3D)
# for more information.
#
# The history of *results* of commands evaluated is not stored by default, but
# can be turned on to be stored with this `.irbrc` setting:
#
# ```
# IRB.conf[:EVAL_HISTORY] = <number>
# ```
#
# See
# [`IRB::Context#eval_history=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-eval_history-3D)
# and History class. The history of command results is not permanently saved in
# any file.
#
# ## Customizing the [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) Prompt
#
# In order to customize the prompt, you can change the following Hash:
#
# ```ruby
# IRB.conf[:PROMPT]
# ```
#
# This example can be used in your `.irbrc`
#
# ```ruby
# IRB.conf[:PROMPT][:MY_PROMPT] = { # name of prompt mode
#   :AUTO_INDENT => false,          # disables auto-indent mode
#   :PROMPT_I =>  ">> ",            # simple prompt
#   :PROMPT_S => nil,               # prompt for continuated strings
#   :PROMPT_C => nil,               # prompt for continuated statement
#   :RETURN => "    ==>%s\n"        # format to return value
# }
#
# IRB.conf[:PROMPT_MODE] = :MY_PROMPT
# ```
#
# Or, invoke irb with the above prompt mode by:
#
# ```
# irb --prompt my-prompt
# ```
#
# Constants `PROMPT_I`, `PROMPT_S` and `PROMPT_C` specify the format. In the
# prompt specification, some special strings are available:
#
# ```
# %N    # command name which is running
# %m    # to_s of main object (self)
# %M    # inspect of main object (self)
# %l    # type of string(", ', /, ]), `]' is inner %w[...]
# %NNi  # indent level. NN is digits and means as same as printf("%NNd").
#       # It can be omitted
# %NNn  # line number.
# %%    # %
# ```
#
# For instance, the default prompt mode is defined as follows:
#
# ```ruby
# IRB.conf[:PROMPT_MODE][:DEFAULT] = {
#   :PROMPT_I => "%N(%m):%03n:%i> ",
#   :PROMPT_N => "%N(%m):%03n:%i> ",
#   :PROMPT_S => "%N(%m):%03n:%i%l ",
#   :PROMPT_C => "%N(%m):%03n:%i* ",
#   :RETURN => "%s\n" # used to printf
# }
# ```
#
# irb comes with a number of available modes:
#
# ```ruby
# # :NULL:
# #   :PROMPT_I:
# #   :PROMPT_N:
# #   :PROMPT_S:
# #   :PROMPT_C:
# #   :RETURN: |
# #     %s
# # :DEFAULT:
# #   :PROMPT_I: ! '%N(%m):%03n:%i> '
# #   :PROMPT_N: ! '%N(%m):%03n:%i> '
# #   :PROMPT_S: ! '%N(%m):%03n:%i%l '
# #   :PROMPT_C: ! '%N(%m):%03n:%i* '
# #   :RETURN: |
# #     => %s
# # :CLASSIC:
# #   :PROMPT_I: ! '%N(%m):%03n:%i> '
# #   :PROMPT_N: ! '%N(%m):%03n:%i> '
# #   :PROMPT_S: ! '%N(%m):%03n:%i%l '
# #   :PROMPT_C: ! '%N(%m):%03n:%i* '
# #   :RETURN: |
# #     %s
# # :SIMPLE:
# #   :PROMPT_I: ! '>> '
# #   :PROMPT_N: ! '>> '
# #   :PROMPT_S:
# #   :PROMPT_C: ! '?> '
# #   :RETURN: |
# #     => %s
# # :INF_RUBY:
# #   :PROMPT_I: ! '%N(%m):%03n:%i> '
# #   :PROMPT_N:
# #   :PROMPT_S:
# #   :PROMPT_C:
# #   :RETURN: |
# #     %s
# #   :AUTO_INDENT: true
# # :XMP:
# #   :PROMPT_I:
# #   :PROMPT_N:
# #   :PROMPT_S:
# #   :PROMPT_C:
# #   :RETURN: |2
# #         ==>%s
# ```
#
# ## Restrictions
#
# Because irb evaluates input immediately after it is syntactically complete,
# the results may be slightly different than directly using Ruby.
#
# ## [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) Sessions
#
# [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) has a special feature,
# that allows you to manage many sessions at once.
#
# You can create new sessions with Irb.irb, and get a list of current sessions
# with the `jobs` command in the prompt.
#
# ### Commands
#
# [`JobManager`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-JobManager)
# provides commands to handle the current sessions:
#
# ```ruby
# jobs    # List of current sessions
# fg      # Switches to the session of the given number
# kill    # Kills the session with the given number
# ```
#
# The `exit` command, or
# [`::irb_exit`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-irb_exit),
# will quit the current session and call any exit hooks with
# [`IRB.irb_at_exit`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-irb_at_exit).
#
# A few commands for loading files within the session are also available:
#
# `source`
# :   Loads a given file in the current session and displays the source lines,
#     see IrbLoader#source\_file
# `irb_load`
# :   Loads the given file similarly to
#     [`Kernel#load`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-load),
#     see IrbLoader#irb\_load
# `irb_require`
# :   Loads the given file similarly to
#     [`Kernel#require`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-require)
#
#
# ### Configuration
#
# The command line options, or
# [`IRB.conf`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-conf),
# specify the default behavior of Irb.irb.
#
# On the other hand, each conf in [Command line options at
# `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Command+line+options)
# is used to individually configure
# [`IRB.irb`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-irb).
#
# If a proc is set for `IRB.conf[:IRB_RC]`, its will be invoked after execution
# of that proc with the context of the current session as its argument. Each
# session can be configured using this mechanism.
#
# ### Session variables
#
# There are a few variables in every Irb session that can come in handy:
#
# `_`
# :   The value command executed, as a local variable
# `__`
# :   The history of evaluated commands. Available only if
#     `IRB.conf[:EVAL_HISTORY]` is not `nil` (which is the default). See also
#     [`IRB::Context#eval_history=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-eval_history-3D)
#     and
#     [`IRB::History`](https://docs.ruby-lang.org/en/2.7.0/IRB/History.html).
# `__[line_no]`
# :   Returns the evaluation value at the given line number, `line_no`. If
#     `line_no` is a negative, the return value `line_no` many lines before the
#     most recent return value.
#
#
# ### Example using [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) Sessions
#
# ```
# # invoke a new session
# irb(main):001:0> irb
# # list open sessions
# irb.1(main):001:0> jobs
#   #0->irb on main (#<Thread:0x400fb7e4> : stop)
#   #1->irb#1 on main (#<Thread:0x40125d64> : running)
#
# # change the active session
# irb.1(main):002:0> fg 0
# # define class Foo in top-level session
# irb(main):002:0> class Foo;end
# # invoke a new session with the context of Foo
# irb(main):003:0> irb Foo
# # define Foo#foo
# irb.2(Foo):001:0> def foo
# irb.2(Foo):002:1>   print 1
# irb.2(Foo):003:1> end
#
# # change the active session
# irb.2(Foo):004:0> fg 0
# # list open sessions
# irb(main):004:0> jobs
#   #0->irb on main (#<Thread:0x400fb7e4> : running)
#   #1->irb#1 on main (#<Thread:0x40125d64> : stop)
#   #2->irb#2 on Foo (#<Thread:0x4011d54c> : stop)
# # check if Foo#foo is available
# irb(main):005:0> Foo.instance_methods #=> [:foo, ...]
#
# # change the active session
# irb(main):006:0> fg 2
# # define Foo#bar in the context of Foo
# irb.2(Foo):005:0> def bar
# irb.2(Foo):006:1>  print "bar"
# irb.2(Foo):007:1> end
# irb.2(Foo):010:0>  Foo.instance_methods #=> [:bar, :foo, ...]
#
# # change the active session
# irb.2(Foo):011:0> fg 0
# irb(main):007:0> f = Foo.new  #=> #<Foo:0x4010af3c>
# # invoke a new session with the context of f (instance of Foo)
# irb(main):008:0> irb f
# # list open sessions
# irb.3(<Foo:0x4010af3c>):001:0> jobs
#   #0->irb on main (#<Thread:0x400fb7e4> : stop)
#   #1->irb#1 on main (#<Thread:0x40125d64> : stop)
#   #2->irb#2 on Foo (#<Thread:0x4011d54c> : stop)
#   #3->irb#3 on #<Foo:0x4010af3c> (#<Thread:0x4010a1e0> : running)
# # evaluate f.foo
# irb.3(<Foo:0x4010af3c>):002:0> foo #=> 1 => nil
# # evaluate f.bar
# irb.3(<Foo:0x4010af3c>):003:0> bar #=> bar => nil
# # kill jobs 1, 2, and 3
# irb.3(<Foo:0x4010af3c>):004:0> kill 1, 2, 3
# # list open sessions, should only include main session
# irb(main):009:0> jobs
#   #0->irb on main (#<Thread:0x400fb7e4> : running)
# # quit irb
# irb(main):010:0> exit
# ```
#
# ```
# save-history.rb -
#     $Release Version: 0.9.6$
#     $Revision$
#     by Keiju ISHITSUKA(keiju@ruby-lang.org)
# ```
#
# --
#
# ```
# frame.rb -
#     $Release Version: 0.9$
#     $Revision$
#     by Keiju ISHITSUKA(Nihon Rational Software Co.,Ltd)
# ```
#
# --
#
# ```
# output-method.rb - output methods used by irb
#     $Release Version: 0.9.6$
#     $Revision$
#     by Keiju ISHITSUKA(keiju@ruby-lang.org)
# ```
#
# --
# DO NOT WRITE ANY MAGIC COMMENT HERE.
module IRB
  # The current
  # [`IRB::Context`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html) of
  # the session, see
  # [`IRB.conf`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-conf)
  #
  # ```
  # irb
  # irb(main):001:0> IRB.CurrentContext.irb_name = "foo"
  # foo(main):002:0> IRB.conf[:MAIN_CONTEXT].irb_name #=> "foo"
  # ```
  def self.CurrentContext; end

  # Displays current configuration.
  #
  # Modifying the configuration is achieved by sending a message to
  # [`IRB.conf`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-conf).
  #
  # See [Configuration at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Configuration)
  # for more information.
  def self.conf; end

  def self.default_src_encoding; end

  # Aborts then interrupts irb.
  #
  # Will raise an Abort exception, or the given `exception`.
  def self.irb_abort(irb, exception = _); end

  # Calls each event hook of `IRB.conf[:TA_EXIT]` when the current session
  # quits.
  def self.irb_at_exit; end

  # Quits irb
  def self.irb_exit(irb, ret); end

  sig { params(ap_path: T.nilable(String), argv: T::Array[String]).void }
  def self.setup(ap_path, argv: ::ARGV); end

  # Initializes [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) and
  # creates a new Irb.irb object at the `TOPLEVEL_BINDING`
  def self.start(ap_path = _); end

  # Returns the current version of
  # [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html), including release
  # version and last updated date.
  def self.version; end
end

# An exception raised by
# [`IRB.irb_abort`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-irb_abort)
class IRB::Abort < ::Exception; end

module IRB::Color
  class << self
    # https://github.com/ruby/irb/blob/d43c3d76/lib/irb/color.rb#L120-L124
    sig { params(text: String, seq: T::Enumerable[T.any(String, Symbol)], colorable: T::Boolean).returns(String) }
    def colorize(text, seq, colorable: false); end
  end
end

# A class that wraps the current state of the irb session, including the
# configuration of
# [`IRB.conf`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-conf).
class IRB::Context
  # Creates a new [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) context.
  #
  # The optional `input_method` argument:
  #
  # `nil`
  # :   uses stdin or Reidline or
  #     [`Readline`](https://docs.ruby-lang.org/en/2.7.0/Readline.html)
  # `String`
  # :   uses a [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
  # `other`
  # :   uses this as InputMethod
  def self.new(irb, workspace = _, input_method = _, output_method = _); end

  # A copy of the default `IRB.conf[:AP_NAME]`
  def ap_name; end

  # A copy of the default `IRB.conf[:AP_NAME]`
  def ap_name=(_); end

  # Can be either the default `IRB.conf[:AUTO_INDENT]`, or the mode set by
  # [`prompt_mode=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-prompt_mode-3D)
  #
  # To disable auto-indentation in irb:
  #
  # ```ruby
  # IRB.conf[:AUTO_INDENT] = false
  # ```
  #
  # or
  #
  # ```ruby
  # irb_context.auto_indent_mode = false
  # ```
  #
  # or
  #
  # ```ruby
  # IRB.CurrentContext.auto_indent_mode = false
  # ```
  #
  # See [Configuration at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Configuration)
  # for more information.
  def auto_indent_mode; end

  # Can be either the default `IRB.conf[:AUTO_INDENT]`, or the mode set by
  # [`prompt_mode=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-prompt_mode-3D)
  #
  # To disable auto-indentation in irb:
  #
  # ```ruby
  # IRB.conf[:AUTO_INDENT] = false
  # ```
  #
  # or
  #
  # ```ruby
  # irb_context.auto_indent_mode = false
  # ```
  #
  # or
  #
  # ```ruby
  # IRB.CurrentContext.auto_indent_mode = false
  # ```
  #
  # See [Configuration at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Configuration)
  # for more information.
  def auto_indent_mode=(_); end

  # The limit of backtrace lines displayed as top `n` and tail `n`.
  #
  # The default value is 16.
  #
  # Can also be set using the `--back-trace-limit` command line option.
  #
  # See [Command line options at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Command+line+options)
  # for more command line options.
  def back_trace_limit; end

  # The limit of backtrace lines displayed as top `n` and tail `n`.
  #
  # The default value is 16.
  #
  # Can also be set using the `--back-trace-limit` command line option.
  #
  # See [Command line options at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Command+line+options)
  # for more command line options.
  def back_trace_limit=(_); end

  # Whether or not debug mode is enabled, see
  # [`debug_level=`](https://docs.ruby-lang.org/en/2.6.0/IRB/Context.html#method-i-debug_level-3D).
  def debug?; end

  # The debug level of irb
  #
  # See
  # [`debug_level=`](https://docs.ruby-lang.org/en/2.6.0/IRB/Context.html#method-i-debug_level-3D)
  # for more information.
  def debug_level; end

  # Sets the debug level of irb
  #
  # Can also be set using the `--irb_debug` command line option.
  #
  # See [Command line options at
  # `IRB`](https://docs.ruby-lang.org/en/2.6.0/IRB.html#label-Command+line+options)
  # for more command line options.
  def debug_level=(value); end

  # Whether to echo the return value to output or not.
  #
  # Uses `IRB.conf[:ECHO]` if available, or defaults to `true`.
  #
  # ```ruby
  # puts "hello"
  # # hello
  # #=> nil
  # IRB.CurrentContext.echo = false
  # puts "omg"
  # # omg
  # ```
  def echo; end

  # Whether to echo the return value to output or not.
  #
  # Uses `IRB.conf[:ECHO]` if available, or defaults to `true`.
  #
  # ```ruby
  # puts "hello"
  # # hello
  # #=> nil
  # IRB.CurrentContext.echo = false
  # puts "omg"
  # # omg
  # ```
  def echo=(_); end

  # Whether to echo the return value to output or not.
  #
  # Uses `IRB.conf[:ECHO]` if available, or defaults to `true`.
  #
  # ```ruby
  # puts "hello"
  # # hello
  # #=> nil
  # IRB.CurrentContext.echo = false
  # puts "omg"
  # # omg
  # ```
  def echo?; end

  # Sets command result history limit. Default value is set from
  # `IRB.conf[:EVAL_HISTORY]`.
  #
  # `no` is an [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) or
  # `nil`.
  #
  # Returns `no` of history items if greater than 0.
  #
  # If `no` is 0, the number of history items is unlimited.
  #
  # If `no` is `nil`, execution result history isn't used (default).
  #
  # History values are available via `__` variable, see
  # [`IRB::History`](https://docs.ruby-lang.org/en/2.7.0/IRB/History.html).
  def eval_history=(*opts, &b); end

  # Exits the current session, see
  # [`IRB.irb_exit`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-irb_exit)
  #
  # Also aliased as:
  # [`__exit__`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-__exit__)
  def exit(ret = _); end

  # Whether
  # [`io`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#attribute-i-io)
  # uses a [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) for the
  # `input_method` passed when creating the current context, see
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-c-new)
  def file_input?; end

  # Whether `^D` (`control-d`) will be ignored or not.
  #
  # If set to `false`, `^D` will quit irb.
  def ignore_eof; end

  # Whether `^D` (`control-d`) will be ignored or not.
  #
  # If set to `false`, `^D` will quit irb.
  def ignore_eof=(_); end

  # Whether `^D` (`control-d`) will be ignored or not.
  #
  # If set to `false`, `^D` will quit irb.
  def ignore_eof?; end

  # Whether `^C` (`control-c`) will be ignored or not.
  #
  # If set to `false`, `^C` will quit irb.
  #
  # If set to `true`,
  #
  # *   during input:   cancel input then return to top level.
  # *   during execute: abandon current execution.
  def ignore_sigint; end

  # Whether `^C` (`control-c`) will be ignored or not.
  #
  # If set to `false`, `^C` will quit irb.
  #
  # If set to `true`,
  #
  # *   during input:   cancel input then return to top level.
  # *   during execute: abandon current execution.
  def ignore_sigint=(_); end

  # Whether `^C` (`control-c`) will be ignored or not.
  #
  # If set to `false`, `^C` will quit irb.
  #
  # If set to `true`,
  #
  # *   during input:   cancel input then return to top level.
  # *   during execute: abandon current execution.
  def ignore_sigint?; end

  # Whether
  # [`inspect_mode`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#attribute-i-inspect_mode)
  # is set or not, see
  # [`inspect_mode=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-inspect_mode-3D)
  # for more detail.
  def inspect?; end

  # A copy of the default `IRB.conf[:INSPECT_MODE]`
  def inspect_mode; end

  # Specifies the inspect mode with `opt`:
  #
  # `true`
  # :   display `inspect`
  # `false`
  # :   display `to_s`
  # `nil`
  # :   inspect mode in non-math mode, non-inspect mode in math mode
  #
  #
  # See
  # [`IRB::Inspector`](https://docs.ruby-lang.org/en/2.7.0/IRB/Inspector.html)
  # for more information.
  #
  # Can also be set using the `--inspect` and `--noinspect` command line
  # options.
  #
  # See [Command line options at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Command+line+options)
  # for more command line options.
  def inspect_mode=(opt); end

  # The current input method
  #
  # Can be either StdioInputMethod, ReadlineInputMethod, ReidlineInputMethod,
  # FileInputMethod or other specified when the context is created. See
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-c-new)
  # for more # information on `input_method`.
  def io; end

  # The current input method
  #
  # Can be either StdioInputMethod, ReadlineInputMethod, ReidlineInputMethod,
  # FileInputMethod or other specified when the context is created. See
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-c-new)
  # for more # information on `input_method`.
  def io=(_); end

  # Current irb session
  def irb; end

  # Current irb session
  def irb=(_); end

  # Can be either name from `IRB.conf[:IRB_NAME]`, or the number of the current
  # job set by JobManager, such as `irb#2`
  def irb_name; end

  # Can be either name from `IRB.conf[:IRB_NAME]`, or the number of the current
  # job set by JobManager, such as `irb#2`
  def irb_name=(_); end

  # Can be either the
  # [`irb_name`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#attribute-i-irb_name)
  # surrounded by parenthesis, or the `input_method` passed to
  # [`Context.new`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-c-new)
  def irb_path; end

  # Can be either the
  # [`irb_name`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#attribute-i-irb_name)
  # surrounded by parenthesis, or the `input_method` passed to
  # [`Context.new`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-c-new)
  def irb_path=(_); end

  # The return value of the last statement evaluated.
  def last_value; end

  # A copy of the default `IRB.conf[:LOAD_MODULES]`
  def load_modules; end

  # A copy of the default `IRB.conf[:LOAD_MODULES]`
  def load_modules=(_); end

  # The top-level workspace, see WorkSpace#main
  def main; end

  # [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) prompt for continuated
  # statement (e.g. immediately after an `if`)
  #
  # See [Customizing the IRB Prompt at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Customizing+the+IRB+Prompt)
  # for more information.
  def prompt_c; end

  # [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) prompt for continuated
  # statement (e.g. immediately after an `if`)
  #
  # See [Customizing the IRB Prompt at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Customizing+the+IRB+Prompt)
  # for more information.
  def prompt_c=(_); end

  # Standard [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) prompt
  #
  # See [Customizing the IRB Prompt at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Customizing+the+IRB+Prompt)
  # for more information.
  def prompt_i; end

  # Standard [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) prompt
  #
  # See [Customizing the IRB Prompt at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Customizing+the+IRB+Prompt)
  # for more information.
  def prompt_i=(_); end

  # A copy of the default `IRB.conf[:PROMPT_MODE]`
  def prompt_mode; end

  # Sets the `mode` of the prompt in this context.
  #
  # See [Customizing the IRB Prompt at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Customizing+the+IRB+Prompt)
  # for more information.
  def prompt_mode=(mode); end

  # See [Customizing the IRB Prompt at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Customizing+the+IRB+Prompt)
  # for more information.
  def prompt_n; end

  # See [Customizing the IRB Prompt at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Customizing+the+IRB+Prompt)
  # for more information.
  def prompt_n=(_); end

  # [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) prompt for continuated
  # strings
  #
  # See [Customizing the IRB Prompt at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Customizing+the+IRB+Prompt)
  # for more information.
  def prompt_s; end

  # [`IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html) prompt for continuated
  # strings
  #
  # See [Customizing the IRB Prompt at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Customizing+the+IRB+Prompt)
  # for more information.
  def prompt_s=(_); end

  # Whether
  # [`verbose?`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-verbose-3F)
  # is `true`, and `input_method` is either StdioInputMethod or
  # ReidlineInputMethod or ReadlineInputMethod, see
  # [`io`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#attribute-i-io)
  # for more information.
  def prompting?; end

  # A copy of the default `IRB.conf[:RC]`
  def rc; end

  # A copy of the default `IRB.conf[:RC]`
  def rc=(_); end

  # A copy of the default `IRB.conf[:RC]`
  def rc?; end

  # The format of the return statement, set by
  # [`prompt_mode=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-prompt_mode-3D)
  # using the `:RETURN` of the `mode` passed to set the current
  # [`prompt_mode`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#attribute-i-prompt_mode).
  def return_format; end

  # The format of the return statement, set by
  # [`prompt_mode=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-prompt_mode-3D)
  # using the `:RETURN` of the `mode` passed to set the current
  # [`prompt_mode`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#attribute-i-prompt_mode).
  def return_format=(_); end

  # Sets `IRB.conf[:SAVE_HISTORY]` to the given `val` and calls
  # init\_save\_history with this context.
  #
  # Will store the number of `val` entries of history in the
  # [`history_file`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-history_file)
  #
  # Add the following to your `.irbrc` to change the number of history entries
  # stored to 1000:
  #
  # ```ruby
  # IRB.conf[:SAVE_HISTORY] = 1000
  # ```
  def save_history=(*opts, &b); end

  # Sets the return value from the last statement evaluated in this context to
  # [`last_value`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#attribute-i-last_value).
  #
  # Also aliased as:
  # [`_set_last_value`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-_set_last_value)
  def set_last_value(value); end

  # The current thread in this context
  def thread; end

  # Sets `IRB.conf[:USE_LOADER]`
  #
  # See
  # [`use_loader`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-use_loader)
  # for more information.
  def use_loader=(*opts, &b); end

  # Whether singleline editor mode is enabled or not.
  #
  # A copy of the default `IRB.conf[:USE_SINGLELINE]`
  def use_readline; end

  # Whether singleline editor mode is enabled or not.
  #
  # A copy of the default `IRB.conf[:USE_SINGLELINE]`
  def use_readline=(opt); end

  # Whether singleline editor mode is enabled or not.
  #
  # A copy of the default `IRB.conf[:USE_SINGLELINE]`
  def use_readline?; end

  # Sets whether or not to use the
  # [`Tracer`](https://docs.ruby-lang.org/en/2.6.0/Tracer.html) library when
  # evaluating statements in this context.
  #
  # See `lib/tracer.rb` for more information.
  def use_tracer=(*opts, &b); end

  # Whether verbose messages are displayed or not.
  #
  # A copy of the default `IRB.conf[:VERBOSE]`
  def verbose; end

  # Whether verbose messages are displayed or not.
  #
  # A copy of the default `IRB.conf[:VERBOSE]`
  def verbose=(_); end

  # Returns whether messages are displayed or not.
  def verbose?; end

  # WorkSpace in the current context
  def workspace; end

  # WorkSpace in the current context
  def workspace=(_); end

  # The toplevel workspace, see
  # [`home_workspace`](https://docs.ruby-lang.org/en/2.7.0/IRB/Context.html#method-i-home_workspace)
  def workspace_home; end
end

# Extends methods for the Context module
module IRB::ContextExtender
  CE = IRB::ContextExtender

  # Evaluate the given `command` from the given `load_file` on the Context
  # module.
  #
  # Will also define any given `aliases` for the method.
  def self.def_extend_command(cmd_name, load_file, *aliases); end

  # Installs the default context extensions as irb commands:
  #
  # Context#eval\_history=
  # :   `irb/ext/history.rb`
  # Context#use\_tracer=
  # :   `irb/ext/tracer.rb`
  # Context#use\_loader=
  # :   `irb/ext/use-loader.rb`
  # Context#save\_history=
  # :   `irb/ext/save-history.rb`
  def self.install_extend_commands; end
end

# Installs the default irb extensions command bundle.
module IRB::ExtendCommandBundle
  # See
  # [`install_alias_method`](https://docs.ruby-lang.org/en/2.7.0/IRB/ExtendCommandBundle.html#method-i-install_alias_method).
  NO_OVERRIDE = T.let(T.unsafe(nil), Integer)

  # See
  # [`install_alias_method`](https://docs.ruby-lang.org/en/2.7.0/IRB/ExtendCommandBundle.html#method-i-install_alias_method).
  OVERRIDE_ALL = T.let(T.unsafe(nil), Integer)

  # See
  # [`install_alias_method`](https://docs.ruby-lang.org/en/2.7.0/IRB/ExtendCommandBundle.html#method-i-install_alias_method).
  OVERRIDE_PRIVATE_ONLY = T.let(T.unsafe(nil), Integer)

  # Installs alias methods for the default irb commands, see
  # [`::install_extend_commands`](https://docs.ruby-lang.org/en/2.7.0/IRB/ExtendCommandBundle.html#method-c-install_extend_commands).
  def install_alias_method(to, from, override = _); end

  # Displays current configuration.
  #
  # Modifying the configuration is achieved by sending a message to
  # [`IRB.conf`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-conf).
  def irb_context; end

  # Quits the current irb context
  #
  # `ret` is the optional signal or message to send to Context#exit
  #
  # Same as `IRB.CurrentContext.exit`.
  def irb_exit(ret = _); end

  # Loads the given file similarly to
  # [`Kernel#load`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-load),
  # see IrbLoader#irb\_load
  def irb_load(*opts, &b); end

  # Loads the given file similarly to
  # [`Kernel#require`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-require)
  def irb_require(*opts, &b); end

  # Evaluate the given `cmd_name` on the given `cmd_class`
  # [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html).
  #
  # Will also define any given `aliases` for the method.
  #
  # The optional `load_file` parameter will be required within the method
  # definition.
  def self.def_extend_command(cmd_name, cmd_class, load_file = _, *aliases); end

  # Installs alias methods for the default irb commands on the given object
  # using
  # [`install_alias_method`](https://docs.ruby-lang.org/en/2.7.0/IRB/ExtendCommandBundle.html#method-i-install_alias_method).
  def self.extend_object(obj); end

  # Installs the default irb commands:
  #
  # `irb_current_working_workspace`
  # :   Context#main
  # `irb_change_workspace`
  # :   Context#change\_workspace
  # `irb_workspaces`
  # :   Context#workspaces
  # `irb_push_workspace`
  # :   Context#push\_workspace
  # `irb_pop_workspace`
  # :   Context#pop\_workspace
  # `irb_load`
  # :   [`irb_load`](https://docs.ruby-lang.org/en/2.7.0/IRB/ExtendCommandBundle.html#method-i-irb_load)
  # `irb_require`
  # :   [`irb_require`](https://docs.ruby-lang.org/en/2.7.0/IRB/ExtendCommandBundle.html#method-i-irb_require)
  # `irb_source`
  # :   IrbLoader#source\_file
  # `irb`
  # :   [`IRB.irb`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#method-c-irb)
  # `irb_jobs`
  # :   JobManager
  # `irb_fg`
  # :   JobManager#switch
  # `irb_kill`
  # :   JobManager#kill
  # `irb_help`
  # :   [Command line options at
  #     `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-Command+line+options)
  def self.install_extend_commands; end
end

# Use a [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) for
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) with irb, see InputMethod
class IRB::FileInputMethod < ::IRB::InputMethod
  # Creates a new input method object
  def self.new(file); end

  # The external encoding for standard input.
  def encoding; end

  # Whether the end of this input method has been reached, returns `true` if
  # there is no more data to read.
  #
  # See [`IO#eof?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof-3F)
  # for more information.
  def eof?; end

  # The file name of this input method, usually given during initialization.
  def file_name; end

  # Reads the next line from this input method.
  #
  # See [`IO#gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets)
  # for more information.
  def gets; end
end

class IRB::InputMethod
  # Creates a new input method object
  def self.new(file = _); end

  # The file name of this input method, usually given during initialization.
  def file_name; end

  # Reads the next line from this input method.
  #
  # See [`IO#gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets)
  # for more information.
  def gets; end

  # The irb prompt associated with this input method
  def prompt; end

  # The irb prompt associated with this input method
  def prompt=(_); end

  # Whether this input method is still readable when there is no more data to
  # read.
  #
  # See [`IO#eof`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof) for
  # more information.
  def readable_after_eof?; end
end

# An irb inspector
#
# In order to create your own custom inspector there are two things you should
# be aware of:
#
# [`Inspector`](https://docs.ruby-lang.org/en/2.7.0/IRB/Inspector.html) uses
# [`inspect_value`](https://docs.ruby-lang.org/en/2.7.0/IRB/Inspector.html#method-i-inspect_value),
# or `inspect_proc`, for output of return values.
#
# This also allows for an optional
# [`init`](https://docs.ruby-lang.org/en/2.7.0/IRB/Inspector.html#method-i-init)+,
# or `init_proc`, which is called when the inspector is activated.
#
# Knowing this, you can create a rudimentary inspector as follows:
#
# ```
# irb(main):001:0> ins = IRB::Inspector.new(proc{ |v| "omg! #{v}" })
# irb(main):001:0> IRB.CurrentContext.inspect_mode = ins # => omg! #<IRB::Inspector:0x007f46f7ba7d28>
# irb(main):001:0> "what?" #=> omg! what?
# ```
class IRB::Inspector
  # Default inspectors available to irb, this includes:
  #
  # `:pp`
  # :   Using
  #     [`Kernel#pretty_inspect`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-pretty_inspect)
  # `:yaml`
  # :   Using YAML.dump
  # `:marshal`
  # :   Using
  #     [`Marshal.dump`](https://docs.ruby-lang.org/en/2.7.0/Marshal.html#method-c-dump)
  INSPECTORS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Creates a new inspector object, using the given `inspect_proc` when output
  # return values in irb.
  def self.new(inspect_proc, init_proc = _); end

  # [`Proc`](https://docs.ruby-lang.org/en/2.7.0/Proc.html) to call when the
  # inspector is activated, good for requiring dependent libraries.
  def init; end

  # [`Proc`](https://docs.ruby-lang.org/en/2.7.0/Proc.html) to call when the
  # input is evaluated and output in irb.
  def inspect_value(v); end

  # Example
  #
  # ```
  # Inspector.def_inspector(key, init_p=nil){|v| v.inspect}
  # Inspector.def_inspector([key1,..], init_p=nil){|v| v.inspect}
  # Inspector.def_inspector(key, inspector)
  # Inspector.def_inspector([key1,...], inspector)
  # ```
  def self.def_inspector(key, arg = _, &block); end

  # Determines the inspector to use where `inspector` is one of the keys passed
  # during inspector definition.
  def self.keys_with_inspector(inspector); end
end

class IRB::Irb
  ATTR_PLAIN = T.let(T.unsafe(nil), String)

  ATTR_TTY = T.let(T.unsafe(nil), String)

  # Creates a new irb session
  def self.new(workspace = _, input_method = _, output_method = _); end

  # Returns the current context of this irb session
  def context; end

  # Evaluates input for this session.
  def eval_input; end

  # Outputs the local variables to this current session, including
  # [`signal_status`](https://docs.ruby-lang.org/en/2.7.0/IRB/Irb.html#method-i-signal_status)
  # and
  # [`context`](https://docs.ruby-lang.org/en/2.7.0/IRB/Irb.html#attribute-i-context),
  # using [`IRB::Locale`](https://docs.ruby-lang.org/en/2.7.0/IRB/Locale.html).
  def inspect; end

  def run(conf = _); end

  # The lexer used by this irb session
  def scanner; end

  # The lexer used by this irb session
  def scanner=(_); end

  # Handler for the signal SIGINT, see
  # [`Kernel#trap`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-trap)
  # for more information.
  def signal_handle; end

  # Evaluates the given block using the given `status`.
  def signal_status(status); end

  # Evaluates the given block using the given `context` as the Context.
  def suspend_context(context); end

  # Evaluates the given block using the given `input_method` as the Context#io.
  #
  # Used by the irb commands `source` and `irb_load`, see [IRB Sessions at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-IRB+Sessions)
  # for more information.
  def suspend_input_method(input_method); end

  # Evaluates the given block using the given `path` as the Context#irb\_path
  # and `name` as the Context#irb\_name.
  #
  # Used by the irb command `source`, see [IRB Sessions at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-IRB+Sessions)
  # for more information.
  def suspend_name(path = _, name = _); end

  # Evaluates the given block using the given `workspace` as the
  # Context#workspace.
  #
  # Used by the irb command `irb_load`, see [IRB Sessions at
  # `IRB`](https://docs.ruby-lang.org/en/2.7.0/IRB.html#module-IRB-label-IRB+Sessions)
  # for more information.
  def suspend_workspace(workspace); end
end

# A convenience module for extending Ruby methods.
module IRB::MethodExtender
  # Extends the given `base_method` with a postfix call to the given
  # `extend_method`.
  def def_post_proc(base_method, extend_method); end

  # Extends the given `base_method` with a prefix call to the given
  # `extend_method`.
  def def_pre_proc(base_method, extend_method); end

  # Returns a unique method name to use as an alias for the given `name`.
  #
  # Usually returns `#{prefix}#{name}#{postfix}<num>`, example:
  #
  # ```ruby
  # new_alias_name('foo') #=> __alias_of__foo__
  # def bar; end
  # new_alias_name('bar') #=> __alias_of__bar__2
  # ```
  def new_alias_name(name, prefix = _, postfix = _); end
end

# An output formatter used internally by the lexer.
module IRB::Notifier
  extend(::Exception2MessageMapper)

  D_NOMSG = T.let(T.unsafe(nil), IRB::Notifier::NoMsgNotifier)

  def Fail(err = _, *rest); end

  def Raise(err = _, *rest); end

  # Define a new
  # [`Notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier.html) output
  # source, returning a new CompositeNotifier with the given `prefix` and
  # `output_method`.
  #
  # The optional `prefix` will be appended to all objects being inspected during
  # output, using the given `output_method` as the output source. If no
  # `output_method` is given, StdioOutputMethod will be used, and all
  # expressions will be sent directly to STDOUT without any additional
  # formatting.
  def self.def_notifier(prefix = _, output_method = _); end

  def self.included(mod); end
end

# An abstract class, or superclass, for CompositeNotifier and LeveledNotifier to
# inherit. It provides several wrapper methods for the OutputMethod object used
# by the [`Notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier.html).
class IRB::Notifier::AbstractNotifier
  # Creates a new
  # [`Notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier.html) object
  def self.new(prefix, base_notifier); end

  # Execute the given block if notifications are enabled.
  def exec_if; end

  # A wrapper method used to determine whether notifications are enabled.
  #
  # Defaults to `true`.
  def notify?; end

  # Same as
  # [`ppx`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/AbstractNotifier.html#method-i-ppx),
  # except it uses the
  # [`prefix`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/AbstractNotifier.html#attribute-i-prefix)
  # given during object initialization. See OutputMethod#ppx for more detail.
  def pp(*objs); end

  # Same as
  # [`pp`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/AbstractNotifier.html#method-i-pp),
  # except it concatenates the given `prefix` with the
  # [`prefix`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/AbstractNotifier.html#attribute-i-prefix)
  # given during object initialization.
  #
  # See OutputMethod#ppx for more detail.
  def ppx(prefix, *objs); end

  # The `prefix` for this
  # [`Notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier.html), which
  # is appended to all objects being inspected during output.
  def prefix; end

  # See OutputMethod#print for more detail.
  def print(*opts); end

  # See OutputMethod#printf for more detail.
  def printf(format, *opts); end

  # See OutputMethod#printn for more detail.
  def printn(*opts); end

  # See OutputMethod#puts for more detail.
  def puts(*objs); end
end

# A class that can be used to create a group of notifier objects with the intent
# of representing a leveled notification system for irb.
#
# This class will allow you to generate other notifiers, and assign them the
# appropriate level for output.
#
# The [`Notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier.html) class
# provides a class-method
# [`Notifier.def_notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier.html#method-c-def_notifier)
# to create a new composite notifier. Using the first composite notifier object
# you create, sibling notifiers can be initialized with
# [`def_notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#method-i-def_notifier).
class IRB::Notifier::CompositeNotifier < ::IRB::Notifier::AbstractNotifier
  # Create a new composite notifier object with the given `prefix`, and
  # `base_notifier` to use for output.
  def self.new(prefix, base_notifier); end

  # Creates a new LeveledNotifier in the composite
  # [`notifiers`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#attribute-i-notifiers)
  # group.
  #
  # The given `prefix` will be assigned to the notifier, and `level` will be
  # used as the index of the
  # [`notifiers`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#attribute-i-notifiers)
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  #
  # This method returns the newly created instance.
  def def_notifier(level, prefix = _); end

  # Returns the leveled notifier for this object
  def level; end

  # Alias for:
  # [`level_notifier=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#method-i-level_notifier-3D)
  def level=(value); end

  # Returns the leveled notifier for this object
  def level_notifier; end

  # Sets the leveled notifier for this object.
  #
  # When the given `value` is an instance of AbstractNotifier,
  # [`level_notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#attribute-i-level_notifier)
  # is set to the given object.
  #
  # When an [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) is
  # given,
  # [`level_notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#attribute-i-level_notifier)
  # is set to the notifier at the index `value` in the
  # [`notifiers`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#attribute-i-notifiers)
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  #
  # If no notifier exists at the index `value` in the
  # [`notifiers`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#attribute-i-notifiers)
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html), an
  # ErrUndefinedNotifier exception is raised.
  #
  # An ErrUnrecognizedLevel exception is raised if the given `value` is not
  # found in the existing
  # [`notifiers`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#attribute-i-notifiers)
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html), or an instance of
  # AbstractNotifier
  #
  # Also aliased as:
  # [`level=`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/CompositeNotifier.html#method-i-level-3D)
  def level_notifier=(value); end

  # List of notifiers in the group
  def notifiers; end
end

class IRB::Notifier::ErrUndefinedNotifier < ::StandardError; end

class IRB::Notifier::ErrUnrecognizedLevel < ::StandardError; end

# A leveled notifier is comparable to the composite group from
# CompositeNotifier#notifiers.
class IRB::Notifier::LeveledNotifier < ::IRB::Notifier::AbstractNotifier
  include(::Comparable)

  # Create a new leveled notifier with the given `base`, and `prefix` to send to
  # AbstractNotifier.new
  #
  # The given `level` is used to compare other leveled notifiers in the
  # CompositeNotifier group to determine whether or not to output notifications.
  def self.new(base, level, prefix); end

  # Compares the level of this notifier object with the given `other` notifier.
  #
  # See the [`Comparable`](https://docs.ruby-lang.org/en/2.7.0/Comparable.html)
  # module for more information.
  def <=>(other); end

  # The current level of this notifier object
  def level; end

  # Whether to output messages to the output method, depending on the level of
  # this notifier object.
  def notify?; end
end

# [`NoMsgNotifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier/NoMsgNotifier.html)
# is a LeveledNotifier that's used as the default notifier when creating a new
# CompositeNotifier.
#
# This notifier is used as the `zero` index, or level `0`, for
# CompositeNotifier#notifiers, and will not output messages of any sort.
class IRB::Notifier::NoMsgNotifier < ::IRB::Notifier::LeveledNotifier
  # Creates a new notifier that should not be used to output messages.
  def self.new; end

  # Ensures notifications are ignored, see AbstractNotifier#notify? for more
  # information.
  def notify?; end
end

# An abstract output class for
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) in irb. This is mainly
# used internally by
# [`IRB::Notifier`](https://docs.ruby-lang.org/en/2.7.0/IRB/Notifier.html). You
# can define your own output method to use with Irb.new, or Context.new
class IRB::OutputMethod
  extend(::Exception2MessageMapper)

  def Fail(err = _, *rest); end

  def Raise(err = _, *rest); end

  # Returns an array of the given `format` and `opts` to be used by
  # [`Kernel#sprintf`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-sprintf),
  # if there was a successful
  # [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) match in the
  # given `format` from
  # [`printf`](https://docs.ruby-lang.org/en/2.7.0/IRB/OutputMethod.html#method-i-printf)
  #
  # ```
  # %
  # <flag>  [#0- +]
  # <minimum field width> (\*|\*[1-9][0-9]*\$|[1-9][0-9]*)
  # <precision>.(\*|\*[1-9][0-9]*\$|[1-9][0-9]*|)?
  # #<length modifier>(hh|h|l|ll|L|q|j|z|t)
  # <conversion specifier>[diouxXeEfgGcsb%]
  # ```
  def parse_printf_format(format, opts); end

  # Prints the given `objs` calling
  # [`Object#inspect`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-inspect)
  # on each.
  #
  # See
  # [`puts`](https://docs.ruby-lang.org/en/2.7.0/IRB/OutputMethod.html#method-i-puts)
  # for more detail.
  def pp(*objs); end

  # Prints the given `objs` calling
  # [`Object#inspect`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-inspect)
  # on each and appending the given `prefix`.
  #
  # See
  # [`puts`](https://docs.ruby-lang.org/en/2.7.0/IRB/OutputMethod.html#method-i-puts)
  # for more detail.
  def ppx(prefix, *objs); end

  # Open this method to implement your own output method, raises a
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html)
  # if you don't define
  # [`print`](https://docs.ruby-lang.org/en/2.7.0/IRB/OutputMethod.html#method-i-print)
  # in your own class.
  def print(*opts); end

  # Extends
  # [`IO#printf`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-printf)
  # to format the given `opts` for
  # [`Kernel#sprintf`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-sprintf)
  # using
  # [`parse_printf_format`](https://docs.ruby-lang.org/en/2.7.0/IRB/OutputMethod.html#method-i-parse_printf_format)
  def printf(format, *opts); end

  # Prints the given `opts`, with a newline delimiter.
  def printn(*opts); end

  # Calls
  # [`print`](https://docs.ruby-lang.org/en/2.7.0/IRB/OutputMethod.html#method-i-print)
  # on each element in the given `objs`, followed by a newline character.
  def puts(*objs); end

  def self.included(mod); end
end

class IRB::OutputMethod::NotImplementedError < ::StandardError; end

class IRB::ReidlineInputMethod < ::IRB::RelineInputMethod; end

class IRB::RelineInputMethod < ::IRB::InputMethod
  include(::Reline)

  # Creates a new input method object using
  # [`Reline`](https://docs.ruby-lang.org/en/3.1.0/Reline.html)
  def self.new; end

  # The external encoding for standard input.
  def encoding; end

  # Whether the end of this input method has been reached, returns `true` if
  # there is no more data to read.
  #
  # See [`IO#eof?`](https://docs.ruby-lang.org/en/3.1.0/IO.html#method-i-eof-3F)
  # for more information.
  def eof?; end

  # Reads the next line from this input method.
  #
  # See [`IO#gets`](https://docs.ruby-lang.org/en/3.1.0/IO.html#method-i-gets)
  # for more information.
  def gets; end

  # Returns the current line number for io.
  #
  # [`line`](https://docs.ruby-lang.org/en/3.1.0/IRB/ReadlineInputMethod.html#method-i-line)
  # counts the number of times
  # [`gets`](https://docs.ruby-lang.org/en/3.1.0/IRB/ReadlineInputMethod.html#method-i-gets)
  # is called.
  #
  # See
  # [`IO#lineno`](https://docs.ruby-lang.org/en/3.1.0/IO.html#method-i-lineno)
  # for more information.
  def line(line_no); end
end

class IRB::ReadlineInputMethod < ::IRB::InputMethod
  include(::Readline)

  # Creates a new input method object using
  # [`Readline`](https://docs.ruby-lang.org/en/2.7.0/Readline.html)
  def self.new; end

  # The external encoding for standard input.
  def encoding; end

  # Whether the end of this input method has been reached, returns `true` if
  # there is no more data to read.
  #
  # See [`IO#eof?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof-3F)
  # for more information.
  def eof?; end

  # Reads the next line from this input method.
  #
  # See [`IO#gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets)
  # for more information.
  def gets; end

  # Returns the current line number for io.
  #
  # [`line`](https://docs.ruby-lang.org/en/2.7.0/IRB/ReadlineInputMethod.html#method-i-line)
  # counts the number of times
  # [`gets`](https://docs.ruby-lang.org/en/2.7.0/IRB/ReadlineInputMethod.html#method-i-gets)
  # is called.
  #
  # See
  # [`IO#lineno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-lineno)
  # for more information.
  def line(line_no); end

  # Whether this input method is still readable when there is no more data to
  # read.
  #
  # See [`IO#eof`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof) for
  # more information.
  def readable_after_eof?; end
end

class IRB::StdioInputMethod < ::IRB::InputMethod
  # Creates a new input method object
  def self.new; end

  # The external encoding for standard input.
  def encoding; end

  # Whether the end of this input method has been reached, returns `true` if
  # there is no more data to read.
  #
  # See [`IO#eof?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof-3F)
  # for more information.
  def eof?; end

  # Reads the next line from this input method.
  #
  # See [`IO#gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets)
  # for more information.
  def gets; end

  # Returns the current line number for io.
  #
  # [`line`](https://docs.ruby-lang.org/en/2.7.0/IRB/StdioInputMethod.html#method-i-line)
  # counts the number of times
  # [`gets`](https://docs.ruby-lang.org/en/2.7.0/IRB/StdioInputMethod.html#method-i-gets)
  # is called.
  #
  # See
  # [`IO#lineno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-lineno)
  # for more information.
  def line(line_no); end

  # Whether this input method is still readable when there is no more data to
  # read.
  #
  # See [`IO#eof`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof) for
  # more information.
  def readable_after_eof?; end
end

# A standard output printer
class IRB::StdioOutputMethod < ::IRB::OutputMethod
  # Prints the given `opts` to standard output, see
  # [`IO#print`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-print) for
  # more information.
  def print(*opts); end
end

class IRB::WorkSpace
  # Creates a new workspace.
  #
  # set self to main if specified, otherwise inherit main from
  # TOPLEVEL\_BINDING.
  def self.new(*main); end

  # The [`Binding`](https://docs.ruby-lang.org/en/2.7.0/Binding.html) of this
  # workspace
  def binding; end

  def code_around_binding; end

  # Evaluate the context of this workspace and use the
  # [`Tracer`](https://docs.ruby-lang.org/en/2.7.0/Tracer.html) library to
  # output the exact lines of code are being executed in chronological order.
  #
  # See `lib/tracer.rb` for more information.
  #
  # Also aliased as:
  # [`__evaluate__`](https://docs.ruby-lang.org/en/2.7.0/IRB/WorkSpace.html#method-i-__evaluate__)
  def evaluate(context, statements, file = _, line = _); end

  # error message manipulator
  def filter_backtrace(bt); end

  # The top-level workspace of this context, also available as
  # `IRB.conf[:__MAIN__]`
  def main; end
end

module IRB::Command
  class << self
    attr_reader :commands

    def register(name, command_class); end
  end
end

class IRB::Command::Base
  class << self
    def category(category = nil); end

    def description(description = nil); end

    def help_message(help_message = nil); end
  end
end

module IRB::HelperMethod
  class << self
    attr_reader :helper_methods

    def register(name, helper_class); end

    def all_helper_methods_info; end
  end
end

class IRB::HelperMethod::Base
  class << self
    def description(description = nil); end
  end
end
