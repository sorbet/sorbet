# typed: __STDLIB_INTERNAL

# The [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html) module
# provides interface for GNU
# [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html). This module
# defines a number of methods to facilitate completion and accesses input
# history from the Ruby interpreter. This module supported Edit Line(libedit)
# too. libedit is compatible with GNU
# [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html).
#
# GNU [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html)
# :   http://www.gnu.org/directory/readline.html
# libedit
# :   http://www.thrysoee.dk/editline/
#
#
# Reads one inputted line with line edit by
# [`Readline.readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-readline)
# method. At this time, the facilitatation completion and the key bind like
# Emacs can be operated like GNU
# [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html).
#
# ```ruby
# require "readline"
# while buf = Readline.readline("> ", true)
#   p buf
# end
# ```
#
# The content that the user input can be recorded to the history. The history
# can be accessed by
# [`Readline::HISTORY`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#HISTORY)
# constant.
#
# ```ruby
# require "readline"
# while buf = Readline.readline("> ", true)
#   p Readline::HISTORY.to_a
#   print("-> ", buf, "\n")
# end
# ```
#
# Documented by Kouji Takao <kouji dot takao at gmail dot com>.
module Readline

  # The [`Object`](https://docs.ruby-lang.org/en/2.6.0/Object.html) with the
  # call method that is a completion for filename. This is sets by
  # [`Readline.completion_proc=`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-completion_proc-3D)
  # method.
  FILENAME_COMPLETION_PROC = T.let(T.unsafe(nil), Object)

  # The history buffer. It extends
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) module,
  # so it behaves just like an array. For example, gets the fifth content that
  # the user input by [HISTORY](4).
  HISTORY = T.let(T.unsafe(nil), Object)

  # The [`Object`](https://docs.ruby-lang.org/en/2.6.0/Object.html) with the
  # call method that is a completion for usernames. This is sets by
  # [`Readline.completion_proc=`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-completion_proc-3D)
  # method.
  USERNAME_COMPLETION_PROC = T.let(T.unsafe(nil), Object)

  # Version string of GNU
  # [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html) or libedit.
  VERSION = T.let(T.unsafe(nil), String)

  # Gets a list of quote characters which can cause a word break.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.basic_quote_characters; end

  # Sets a list of quote characters which can cause a word break.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.basic_quote_characters=(_); end

  # Gets the basic list of characters that signal a break between words for the
  # completer routine.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.basic_word_break_characters; end

  # Sets the basic list of characters that signal a break between words for the
  # completer routine. The default is the characters which break words for
  # completion in Bash: " t\\n\\"\\\\'`@$><=;|&{(".
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.basic_word_break_characters=(_); end

  # Gets a list of characters which can be used to quote a substring of the
  # line.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.completer_quote_characters; end

  # Sets a list of characters which can be used to quote a substring of the
  # line. Completion occurs on the entire substring, and within the substring
  # [`Readline.completer_word_break_characters`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-completer_word_break_characters)
  # are treated as any other character, unless they also appear within this
  # list.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.completer_quote_characters=(_); end

  # Gets the basic list of characters that signal a break between words for
  # rl\_complete\_internal().
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.completer_word_break_characters; end

  # Sets the basic list of characters that signal a break between words for
  # rl\_complete\_internal(). The default is the value of
  # [`Readline.basic_word_break_characters`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-basic_word_break_characters).
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.completer_word_break_characters=(_); end

  # Returns a string containing a character to be appended on completion. The
  # default is a space (" ").
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.completion_append_character; end

  # Specifies a character to be appended on completion. Nothing will be appended
  # if an empty string ("") or nil is specified.
  #
  # For example:
  #
  # ```ruby
  # require "readline"
  #
  # Readline.readline("> ", true)
  # Readline.completion_append_character = " "
  # ```
  #
  # Result:
  #
  # ```
  # >
  # Input "/var/li".
  #
  # > /var/li
  # Press TAB key.
  #
  # > /var/lib
  # Completes "b" and appends " ". So, you can continuously input "/usr".
  #
  # > /var/lib /usr
  # ```
  #
  # NOTE: Only one character can be specified. When "string" is specified, sets
  # only "s" that is the first.
  #
  # ```ruby
  # require "readline"
  #
  # Readline.completion_append_character = "string"
  # p Readline.completion_append_character # => "s"
  # ```
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.completion_append_character=(_); end

  # Returns true if completion ignores case. If no, returns false.
  #
  # NOTE: Returns the same object that is specified by
  # [`Readline.completion_case_fold=`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-completion_case_fold-3D)
  # method.
  #
  # ```ruby
  # require "readline"
  #
  # Readline.completion_case_fold = "This is a String."
  # p Readline.completion_case_fold # => "This is a String."
  # ```
  def self.completion_case_fold; end

  # Sets whether or not to ignore case on completion.
  def self.completion_case_fold=(_); end

  # Returns the completion
  # [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object.
  def self.completion_proc; end

  # Specifies a [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object
  # `proc` to determine completion behavior. It should take input string and
  # return an array of completion candidates.
  #
  # The default completion is used if `proc` is nil.
  #
  # The [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) that is
  # passed to the [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html)
  # depends on the
  # [`Readline.completer_word_break_characters`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-completer_word_break_characters)
  # property. By default the word under the cursor is passed to the
  # [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html). For example, if the
  # input is "foo bar" then only "bar" would be passed to the completion
  # [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html).
  #
  # Upon successful completion the
  # [`Readline.completion_append_character`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-completion_append_character)
  # will be appended to the input so the user can start working on their next
  # argument.
  #
  # # Examples
  #
  # ## Completion for a Static List
  #
  # ```ruby
  # require 'readline'
  #
  # LIST = [
  #   'search', 'download', 'open',
  #   'help', 'history', 'quit',
  #   'url', 'next', 'clear',
  #   'prev', 'past'
  # ].sort
  #
  # comp = proc { |s| LIST.grep(/^#{Regexp.escape(s)}/) }
  #
  # Readline.completion_append_character = " "
  # Readline.completion_proc = comp
  #
  # while line = Readline.readline('> ', true)
  #   p line
  # end
  # ```
  #
  # ## Completion For Directory Contents
  #
  # ```ruby
  # require 'readline'
  #
  # Readline.completion_append_character = " "
  # Readline.completion_proc = Proc.new do |str|
  #   Dir[str+'*'].grep(/^#{Regexp.escape(str)}/)
  # end
  #
  # while line = Readline.readline('> ', true)
  #   p line
  # end
  # ```
  #
  # # Autocomplete strategies
  #
  # When working with auto-complete there are some strategies that work well. To
  # get some ideas you can take a look at the
  # [completion.rb](https://svn.ruby-lang.org/repos/ruby/trunk/lib/irb/completion.rb)
  # file for irb.
  #
  # The common strategy is to take a list of possible completions and filter it
  # down to those completions that start with the user input. In the above
  # examples
  # [`Enumerator.grep`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-grep)
  # is used. The input is escaped to prevent
  # [`Regexp`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html) special
  # characters from interfering with the matching.
  #
  # It may also be helpful to use the
  # [`Abbrev`](https://docs.ruby-lang.org/en/2.6.0/Abbrev.html) library to
  # generate completions.
  #
  # Raises
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html) if
  # `proc` does not respond to the call method.
  def self.completion_proc=(_); end

  # Delete text between start and end in the current line.
  #
  # See GNU Readline's rl\_delete\_text function.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.delete_text(*_); end

  # Specifies Emacs editing mode. The default is this mode. See the manual of
  # GNU [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html) for
  # details of Emacs editing mode.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.emacs_editing_mode; end

  # Returns true if emacs mode is active. Returns false if not.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.emacs_editing_mode?; end

  # Gets a list of characters that cause a filename to be quoted by the
  # completer when they appear in a completed filename.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.filename_quote_characters; end

  # Sets a list of characters that cause a filename to be quoted by the
  # completer when they appear in a completed filename. The default is nil.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.filename_quote_characters=(_); end

  # Returns the terminal's rows and columns.
  #
  # See GNU Readline's rl\_get\_screen\_size function.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.get_screen_size; end

  # Specifies a [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) object
  # `input` that is input stream for
  # [`Readline.readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-readline)
  # method.
  def self.input=(_); end

  # Insert text into the line at the current cursor position.
  #
  # See GNU Readline's rl\_insert\_text function.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.insert_text(_); end

  # Returns the full line that is being edited. This is useful from within the
  # complete\_proc for determining the context of the completion request.
  #
  # The length of `Readline.line_buffer` and GNU Readline's rl\_end are same.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.line_buffer; end

  # Specifies a [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) object
  # `output` that is output stream for
  # [`Readline.readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-readline)
  # method.
  def self.output=(_); end

  # Returns the index of the current cursor position in `Readline.line_buffer`.
  #
  # The index in `Readline.line_buffer` which matches the start of input-string
  # passed to
  # [`completion_proc`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#method-c-completion_proc)
  # is computed by subtracting the length of input-string from `Readline.point`.
  #
  # ```ruby
  # start = (the length of input-string) - Readline.point
  # ```
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.point; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the index of the
  # current cursor position in `Readline.line_buffer`.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  #
  # See `Readline.point`.
  def self.point=(_); end

  # Returns a [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object
  # `proc` to call after the first prompt has been printed and just before
  # readline starts reading input characters. The default is nil.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.pre_input_hook; end

  # Specifies a [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object
  # `proc` to call after the first prompt has been printed and just before
  # readline starts reading input characters.
  #
  # See GNU Readline's rl\_pre\_input\_hook variable.
  #
  # Raises
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html) if
  # `proc` does not respond to the call method.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.pre_input_hook=(_); end

  # Returns the quoting detection
  # [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object.
  def self.quoting_detection_proc; end

  # Specifies a [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html) object
  # `proc` to determine if a character in the user's input is escaped. It should
  # take the user's input and the index of the character in question as input,
  # and return a boolean (true if the specified character is escaped).
  #
  # [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html) will only
  # call this proc with characters specified in `completer_quote_characters`, to
  # discover if they indicate the end of a quoted argument, or characters
  # specified in `completer_word_break_characters`, to discover if they indicate
  # a break between arguments.
  #
  # If `completer_quote_characters` is not set, or if the user input doesn't
  # contain one of the `completer_quote_characters` or a ++ character,
  # [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html) will not
  # attempt to use this proc at all.
  #
  # Raises
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html) if
  # `proc` does not respond to the call method.
  def self.quoting_detection_proc=(_); end

  # Shows the `prompt` and reads the inputted line with line editing. The
  # inputted line is added to the history if `add_hist` is true.
  #
  # Returns nil when the inputted line is empty and user inputs EOF (Presses ^D
  # on UNIX).
  #
  # Raises [`IOError`](https://docs.ruby-lang.org/en/2.6.0/IOError.html)
  # exception if one of below conditions are satisfied.
  # 1.  stdin was closed.
  # 2.  stdout was closed.
  #
  #
  # This method supports thread. Switches the thread context when waits
  # inputting line.
  #
  # Supports line edit when inputs line. Provides VI and Emacs editing mode.
  # Default is Emacs editing mode.
  #
  # NOTE: Terminates ruby interpreter and does not return the terminal status
  # after user pressed '^C' when wait inputting line. Give 3 examples that avoid
  # it.
  #
  # *   Catches the
  #     [`Interrupt`](https://docs.ruby-lang.org/en/2.6.0/Interrupt.html)
  #     exception by pressed ^C after returns terminal status:
  #
  # ```
  # require "readline"
  #
  # stty_save = `stty -g`.chomp
  # begin
  #   while buf = Readline.readline
  #       p buf
  #       end
  #     rescue Interrupt
  #       system("stty", stty_save)
  #       exit
  #     end
  #   end
  # end
  # ```
  #
  # *   Catches the INT signal by pressed ^C after returns terminal status:
  #
  # ```ruby
  # require "readline"
  #
  # stty_save = `stty -g`.chomp
  # trap("INT") { system "stty", stty_save; exit }
  #
  # while buf = Readline.readline
  #   p buf
  # end
  # ```
  #
  # *   Ignores pressing ^C:
  #
  # ```ruby
  # require "readline"
  #
  # trap("INT", "SIG_IGN")
  #
  # while buf = Readline.readline
  #   p buf
  # end
  # ```
  #
  #
  # Can make as follows with
  # [`Readline::HISTORY`](https://docs.ruby-lang.org/en/2.6.0/Readline.html#HISTORY)
  # constant. It does not record to the history if the inputted line is empty or
  # the same it as last one.
  #
  # ```ruby
  # require "readline"
  #
  # while buf = Readline.readline("> ", true)
  #   # p Readline::HISTORY.to_a
  #   Readline::HISTORY.pop if /^\s*$/ =~ buf
  #
  #   begin
  #     if Readline::HISTORY[Readline::HISTORY.length-2] == buf
  #       Readline::HISTORY.pop
  #     end
  #   rescue IndexError
  #   end
  #
  #   # p Readline::HISTORY.to_a
  #   print "-> ", buf, "\n"
  # end
  # ```
  def self.readline(*_); end

  # Change what's displayed on the screen to reflect the current contents.
  #
  # See GNU Readline's rl\_redisplay function.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.redisplay; end

  # Clear the current input line.
  def self.refresh_line; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) terminal size to
  # `rows` and `columns`.
  #
  # See GNU Readline's rl\_set\_screen\_size function.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.set_screen_size(_, _); end

  # Gets the list of characters that are word break characters, but should be
  # left in text when it is passed to the completion function.
  #
  # See GNU Readline's rl\_special\_prefixes variable.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.special_prefixes; end

  # Sets the list of characters that are word break characters, but should be
  # left in text when it is passed to the completion function. Programs can use
  # this to help determine what kind of completing to do. For instance, Bash
  # sets this variable to "$@" so that it can complete shell variables and
  # hostnames.
  #
  # See GNU Readline's rl\_special\_prefixes variable.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.special_prefixes=(_); end

  # Specifies VI editing mode. See the manual of GNU
  # [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html) for details
  # of VI editing mode.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.vi_editing_mode; end

  # Returns true if vi mode is active. Returns false if not.
  #
  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.6.0/NotImplementedError.html)
  # if the using readline library does not support.
  def self.vi_editing_mode?; end
end
