# typed: __STDLIB_INTERNAL

# ## Manipulates strings like the UNIX Bourne shell
#
# This module manipulates strings according to the word parsing rules of the
# UNIX Bourne shell.
#
# The shellwords() function was originally a port of shellwords.pl, but modified
# to conform to the [`Shell`](https://docs.ruby-lang.org/en/2.6.0/Shell.html) &
# Utilities volume of the IEEE Std 1003.1-2008, 2016 Edition [1].
#
# ### Usage
#
# You can use
# [`Shellwords`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html) to parse a
# string into a Bourne shell friendly
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html).
#
# ```ruby
# require 'shellwords'
#
# argv = Shellwords.split('three blind "mice"')
# argv #=> ["three", "blind", "mice"]
# ```
#
# Once you've required
# [`Shellwords`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html), you can
# use the split alias
# [`String#shellsplit`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-shellsplit).
#
# ```ruby
# argv = "see how they run".shellsplit
# argv #=> ["see", "how", "they", "run"]
# ```
#
# Be careful you don't leave a quote unmatched.
#
# ```ruby
# argv = "they all ran after the farmer's wife".shellsplit
#      #=> ArgumentError: Unmatched double quote: ...
# ```
#
# In this case, you might want to use
# [`Shellwords.escape`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html#method-c-escape),
# or its alias
# [`String#shellescape`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-shellescape).
#
# This method will escape the
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) for you to safely
# use with a Bourne shell.
#
# ```ruby
# argv = Shellwords.escape("special's.txt")
# argv #=> "special\\'s.txt"
# system("cat " + argv)
# ```
#
# [`Shellwords`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html) also comes
# with a core extension for
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html),
# [`Array#shelljoin`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-shelljoin).
#
# ```ruby
# argv = %w{ls -lta lib}
# system(argv.shelljoin)
# ```
#
# You can use this method to create an escaped string out of an array of tokens
# separated by a space. In this example we used the literal shortcut for
# Array.new.
#
# ### Authors
# *   Wakou Aoyama
# *   Akinori MUSHA <knu@iDaemons.org>
#
#
# ### Contact
# *   Akinori MUSHA <knu@iDaemons.org> (current maintainer)
#
#
# ### Resources
#
# 1: [IEEE Std 1003.1-2008, 2016 Edition, the Shell & Utilities
# volume](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/contents.html)
module Shellwords
  # Alias for:
  # [`shellescape`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html#method-c-shellescape)
  sig { params(str: String).returns(String) }
  def self.escape(str); end

  # Alias for:
  # [`shelljoin`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html#method-c-shelljoin)
  sig { params(str: T::Array[String]).returns(String) }
  def self.join(str); end

  # Escapes a string so that it can be safely used in a Bourne shell command
  # line. `str` can be a non-string object that responds to `to_s`.
  #
  # Note that a resulted string should be used unquoted and is not intended for
  # use in double quotes nor in single quotes.
  #
  # ```ruby
  # argv = Shellwords.escape("It's better to give than to receive")
  # argv #=> "It\\'s\\ better\\ to\\ give\\ than\\ to\\ receive"
  # ```
  #
  # [`String#shellescape`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-shellescape)
  # is a shorthand for this function.
  #
  # ```ruby
  # argv = "It's better to give than to receive".shellescape
  # argv #=> "It\\'s\\ better\\ to\\ give\\ than\\ to\\ receive"
  #
  # # Search files in lib for method definitions
  # pattern = "^[ \t]*def "
  # open("| grep -Ern #{pattern.shellescape} lib") { |grep|
  #   grep.each_line { |line|
  #     file, lineno, matched_line = line.split(':', 3)
  #     # ...
  #   }
  # }
  # ```
  #
  # It is the caller's responsibility to encode the string in the right encoding
  # for the shell environment where this string is used.
  #
  # Multibyte characters are treated as multibyte characters, not as bytes.
  #
  # Returns an empty quoted
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) if `str` has a
  # length of zero.
  #
  # Also aliased as:
  # [`escape`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html#method-c-escape)
  sig { params(str: String).returns(String) }
  module_function def shellescape(str); end
  
  # Builds a command line string from an argument list, `array`.
  #
  # All elements are joined into a single string with fields separated by a
  # space, where each element is escaped for the Bourne shell and stringified
  # using `to_s`.
  #
  # ```ruby
  # ary = ["There's", "a", "time", "and", "place", "for", "everything"]
  # argv = Shellwords.join(ary)
  # argv #=> "There\\'s a time and place for everything"
  # ```
  #
  # [`Array#shelljoin`](https://docs.ruby-lang.org/en/2.6.0/Array.html#method-i-shelljoin)
  # is a shortcut for this function.
  #
  # ```ruby
  # ary = ["Don't", "rock", "the", "boat"]
  # argv = ary.shelljoin
  # argv #=> "Don\\'t rock the boat"
  # ```
  #
  # You can also mix non-string objects in the elements as allowed in
  # Array#join.
  #
  # ```ruby
  # output = `#{['ps', '-p', $$].shelljoin}`
  # ```
  #
  #
  # Also aliased as:
  # [`join`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html#method-c-join)
  sig { params(str: T::Array[String]).returns(String) }
  module_function def shelljoin(str); end

  # Splits a string into an array of tokens in the same way the UNIX Bourne
  # shell does.
  #
  # ```ruby
  # argv = Shellwords.split('here are "two words"')
  # argv #=> ["here", "are", "two words"]
  # ```
  #
  # Note, however, that this is not a command line parser.
  # [`Shell`](https://docs.ruby-lang.org/en/2.6.0/Shell.html) metacharacters
  # except for the single and double quotes and backslash are not treated as
  # such.
  #
  # ```ruby
  # argv = Shellwords.split('ruby my_prog.rb | less')
  # argv #=> ["ruby", "my_prog.rb", "|", "less"]
  # ```
  #
  # [`String#shellsplit`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-shellsplit)
  # is a shortcut for this function.
  #
  # ```ruby
  # argv = 'here are "two words"'.shellsplit
  # argv #=> ["here", "are", "two words"]
  # ```
  #
  #
  # Also aliased as:
  # [`shellwords`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html#method-c-shellwords),
  # [`split`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html#method-c-split)
  sig { params(str: String).returns(T::Array[String]) }
  module_function def shellsplit(str); end

  # Alias for:
  # [`shellsplit`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html#method-i-shellsplit)
  sig { params(str: String).returns(T::Array[String]) }
  module_function def shellwords(str); end

  # Alias for:
  # [`shellsplit`](https://docs.ruby-lang.org/en/2.6.0/Shellwords.html#method-c-shellsplit)
  sig { params(str: String).returns(T::Array[String]) }
  def self.split(str); end
end

class String
  # call-seq:
  #   str.shellsplit => array
  #
  # Splits +str+ into an array of tokens in the same way the UNIX
  # Bourne shell does.
  #
  # See Shellwords.shellsplit for details.
  sig { returns(T::Array[String]) }
  def shellsplit; end

  # call-seq:
  #   str.shellescape => string
  #
  # Escapes +str+ so that it can be safely used in a Bourne shell
  # command line.
  #
  # See Shellwords.shellescape for details.
  sig { returns(String) }
  def shellescape; end
end

class Array
  # call-seq:
  #   array.shelljoin => string
  #
  # Builds a command line string from an argument list +array+ joining
  # all elements escaped for the Bourne shell and separated by a space.
  #
  # See Shellwords.shelljoin for details.
  sig { returns(String) }
  def shelljoin; end
end
