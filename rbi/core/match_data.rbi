# typed: __STDLIB_INTERNAL

# [`MatchData`](https://docs.ruby-lang.org/en/2.7.0/MatchData.html) encapsulates
# the result of matching a
# [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) against string. It
# is returned by
# [`Regexp#match`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-i-match)
# and
# [`String#match`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-match),
# and also stored in a global variable returned by
# [`Regexp.last_match`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-c-last_match).
#
# Usage:
#
# ```ruby
# url = 'https://docs.ruby-lang.org/en/2.5.0/MatchData.html'
# m = url.match(/(\d\.?)+/)   # => #<MatchData "2.5.0" 1:"0">
# m.string                    # => "https://docs.ruby-lang.org/en/2.5.0/MatchData.html"
# m.regexp                    # => /(\d\.?)+/
# # entire matched substring:
# m[0]                        # => "2.5.0"
#
# # Working with unnamed captures
# m = url.match(%r{([^/]+)/([^/]+)\.html$})
# m.captures                  # => ["2.5.0", "MatchData"]
# m[1]                        # => "2.5.0"
# m.values_at(1, 2)           # => ["2.5.0", "MatchData"]
#
# # Working with named captures
# m = url.match(%r{(?<version>[^/]+)/(?<module>[^/]+)\.html$})
# m.captures                  # => ["2.5.0", "MatchData"]
# m.named_captures            # => {"version"=>"2.5.0", "module"=>"MatchData"}
# m[:version]                 # => "2.5.0"
# m.values_at(:version, :module)
#                             # => ["2.5.0", "MatchData"]
# # Numerical indexes are working, too
# m[1]                        # => "2.5.0"
# m.values_at(1, 2)           # => ["2.5.0", "MatchData"]
# ```
#
# ## Global variables equivalence
#
# Parts of last
# [`MatchData`](https://docs.ruby-lang.org/en/2.7.0/MatchData.html) (returned by
# [`Regexp.last_match`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-c-last_match))
# are also aliased as global variables:
#
# *   `$~` is
#     [`Regexp.last_match`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-c-last_match);
# *   `$&` is
#     [`[Regexp.last_match](0)`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-c-last_match);
# *   `$1`, `$2`, and so on are
#     [`[Regexp.last_match](i)`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-c-last_match)
#     (captures by number);
# *   `$`` is
#     [`Regexp.last_match`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-c-last_match)`.pre_match`;
# *   `$'` is
#     [`Regexp.last_match`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-c-last_match)`.post_match`;
# *   `$+` is
#     [`[Regexp.last_match](-1)`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html#method-c-last_match)
#     (the last capture).
#
#
# See also "Special global variables" section in
# [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html) documentation.
class MatchData < Object
  # Equality---Two matchdata are equal if their target strings, patterns, and
  # matched positions are identical.
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Match Reference --
  # [`MatchData`](https://docs.ruby-lang.org/en/2.7.0/MatchData.html) acts as an
  # array, and may be accessed using the normal array indexing techniques.
  # `mtch[0]` is equivalent to the special variable `$&`, and returns the entire
  # matched string. `mtch[1]`, `mtch[2]`, and so on return the values of the
  # matched backreferences (portions of the pattern between parentheses).
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m          #=> #<MatchData "HX1138" 1:"H" 2:"X" 3:"113" 4:"8">
  # m[0]       #=> "HX1138"
  # m[1, 2]    #=> ["H", "X"]
  # m[1..3]    #=> ["H", "X", "113"]
  # m[-3, 2]   #=> ["X", "113"]
  #
  # m = /(?<foo>a+)b/.match("ccaaab")
  # m          #=> #<MatchData "aaab" foo:"aaa">
  # m["foo"]   #=> "aaa"
  # m[:foo]    #=> "aaa"
  # ```
  sig do
    params(
        i_or_start_or_range_or_name: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        i_or_start_or_range_or_name: Integer,
        length: Integer,
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        i_or_start_or_range_or_name: T::Range[Integer],
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        i_or_start_or_range_or_name: T.any(String, Symbol),
    )
    .returns(T.nilable(String))
  end
  def [](i_or_start_or_range_or_name, length=T.unsafe(nil)); end

  # Returns the offset of the start of the *n*th element of the match array in
  # the string. *n* can be a string or symbol to reference a named capture.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.begin(0)       #=> 1
  # m.begin(2)       #=> 2
  #
  # m = /(?<foo>.)(.)(?<bar>.)/.match("hoge")
  # p m.begin(:foo)  #=> 0
  # p m.begin(:bar)  #=> 2
  # ```
  sig do
    params(
        n: T.any(Integer, Symbol, String),
    )
    .returns(Integer)
  end
  def begin(n); end

  # Returns a two-element array containing the beginning and ending byte-based
  # offsets of the nth match. n can be a string or symbol to reference a named capture.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.byteoffset(0)      #=> [1, 7]
  # m.byteoffset(4)      #=> [6, 7]
  #
  # m = /(?<foo>.)(.)(?<bar>.)/.match("hoge")
  # m.byteoffset(:foo)   #=> [0, 1]
  # m.byteoffset(:bar)   #=> [2, 3]
  # ```
  sig do
    params(
      n: T.any(Integer, Symbol, String),
    )
    .returns(T::Array[Integer])
  end
  def byteoffset(n); end

  # Returns the array of captures; equivalent to `mtch.to_a[1..-1]`.
  #
  # ```ruby
  # f1,f2,f3,f4 = /(.)(.)(\d+)(\d)/.match("THX1138.").captures
  # f1    #=> "H"
  # f2    #=> "X"
  # f3    #=> "113"
  # f4    #=> "8"
  # ```
  sig {returns(T::Array[String])}
  def captures(); end

  # Returns the offset of the character immediately following the end of the
  # *n*th element of the match array in the string. *n* can be a string or
  # symbol to reference a named capture.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.end(0)         #=> 7
  # m.end(2)         #=> 3
  #
  # m = /(?<foo>.)(.)(?<bar>.)/.match("hoge")
  # p m.end(:foo)    #=> 1
  # p m.end(:bar)    #=> 3
  # ```
  sig do
    params(
        n: T.any(Integer, Symbol, String),
    )
    .returns(Integer)
  end
  def end(n); end

  # Equality---Two matchdata are equal if their target strings, patterns, and
  # matched positions are identical.
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(other); end

  # Produce a hash based on the target string, regexp and matched positions of
  # this matchdata.
  #
  # See also
  # [`Object#hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash).
  sig {returns(Integer)}
  def hash(); end

  # Returns a printable version of *mtch*.
  #
  # ```ruby
  # puts /.$/.match("foo").inspect
  # #=> #<MatchData "o">
  #
  # puts /(.)(.)(.)/.match("foo").inspect
  # #=> #<MatchData "foo" 1:"f" 2:"o" 3:"o">
  #
  # puts /(.)(.)?(.)/.match("fo").inspect
  # #=> #<MatchData "fo" 1:"f" 2:nil 3:"o">
  #
  # puts /(?<foo>.)(?<bar>.)(?<baz>.)/.match("hoge").inspect
  # #=> #<MatchData "hog" foo:"h" bar:"o" baz:"g">
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns the number of elements in the match array.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.length   #=> 5
  # m.size     #=> 5
  # ```
  sig {returns(Integer)}
  def length(); end

  # Returns a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) using
  # named capture.
  #
  # A key of the hash is a name of the named captures. A value of the hash is a
  # string of last successful capture of corresponding group.
  #
  # ```ruby
  # m = /(?<a>.)(?<b>.)/.match("01")
  # m.named_captures #=> {"a" => "0", "b" => "1"}
  #
  # m = /(?<a>.)(?<b>.)?/.match("0")
  # m.named_captures #=> {"a" => "0", "b" => nil}
  #
  # m = /(?<a>.)(?<a>.)/.match("01")
  # m.named_captures #=> {"a" => "1"}
  #
  # m = /(?<a>x)|(?<a>y)/.match("x")
  # m.named_captures #=> {"a" => "x"}
  # ```
  sig {returns(T::Hash[String, T.nilable(String)])}
  def named_captures(); end

  # Returns a list of names of captures as an array of strings. It is same as
  # mtch.regexp.names.
  #
  # ```ruby
  # /(?<foo>.)(?<bar>.)(?<baz>.)/.match("hoge").names
  # #=> ["foo", "bar", "baz"]
  #
  # m = /(?<x>.)(?<y>.)?/.match("a") #=> #<MatchData "a" x:"a" y:nil>
  # m.names                          #=> ["x", "y"]
  # ```
  sig {returns(T::Array[String])}
  def names(); end

  # Returns a two-element array containing the beginning and ending offsets of
  # the *n*th match. *n* can be a string or symbol to reference a named capture.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.offset(0)      #=> [1, 7]
  # m.offset(4)      #=> [6, 7]
  #
  # m = /(?<foo>.)(.)(?<bar>.)/.match("hoge")
  # p m.offset(:foo) #=> [0, 1]
  # p m.offset(:bar) #=> [2, 3]
  # ```
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Integer])
  end
  def offset(n); end

  # Returns the portion of the original string after the current match.
  # Equivalent to the special variable `$'`.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138: The Movie")
  # m.post_match   #=> ": The Movie"
  # ```
  sig {returns(String)}
  def post_match(); end

  # Returns the portion of the original string before the current match.
  # Equivalent to the special variable `$``.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.pre_match   #=> "T"
  # ```
  sig {returns(String)}
  def pre_match(); end

  # Returns the regexp.
  #
  # ```ruby
  # m = /a.*b/.match("abc")
  # m.regexp #=> /a.*b/
  # ```
  sig {returns(Regexp)}
  def regexp(); end

  # Returns the number of elements in the match array.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.length   #=> 5
  # m.size     #=> 5
  # ```
  sig {returns(Integer)}
  def size(); end

  # Returns a frozen copy of the string passed in to `match`.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.string   #=> "THX1138."
  # ```
  sig {returns(String)}
  def string(); end

  # Returns the array of matches.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.to_a   #=> ["HX1138", "H", "X", "113", "8"]
  # ```
  #
  # Because `to_a` is called when expanding `*`*variable*, there's a useful
  # assignment shortcut for extracting matched fields. This is slightly slower
  # than accessing the fields directly (as an intermediate array is generated).
  #
  # ```ruby
  # all,f1,f2,f3 = * /(.)(.)(\d+)(\d)/.match("THX1138.")
  # all   #=> "HX1138"
  # f1    #=> "H"
  # f2    #=> "X"
  # f3    #=> "113"
  # ```
  sig {returns(T::Array[String])}
  def to_a(); end

  # Returns the entire matched string.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138.")
  # m.to_s   #=> "HX1138"
  # ```
  sig {returns(String)}
  def to_s(); end

  # Uses each *index* to access the matching values, returning an array of the
  # corresponding matches.
  #
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138: The Movie")
  # m.to_a               #=> ["HX1138", "H", "X", "113", "8"]
  # m.values_at(0, 2, -2)   #=> ["HX1138", "X", "113"]
  #
  # m = /(?<a>\d+) *(?<op>[+\-*\/]) *(?<b>\d+)/.match("1 + 2")
  # m.to_a               #=> ["1 + 2", "1", "+", "2"]
  # m.values_at(:a, :b, :op) #=> ["1", "2", "+"]
  # ```
  sig do
    params(
        indexes: T.any(Integer, Symbol),
    )
    .returns(T::Array[String])
  end
  def values_at(*indexes); end
end
