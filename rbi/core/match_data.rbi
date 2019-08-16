# typed: __STDLIB_INTERNAL

class MatchData < Object
  # Equality—Two matchdata are equal if their target strings, patterns, and
  # matched positions are identical.
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

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

  sig do
    params(
        n: Integer,
    )
    .returns(Integer)
  end
  def begin(n); end

  # Returns the array of captures; equivalent to `mtch.to_a[1..-1]` .
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

  sig do
    params(
        n: Integer,
    )
    .returns(Integer)
  end
  def end(n); end

  # Equality—Two matchdata are equal if their target strings, patterns, and
  # matched positions are identical.
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(other); end

  # Produce a hash based on the target string, regexp and matched positions
  # of this matchdata.
  # 
  # See also Object\#hash.
  sig {returns(Integer)}
  def hash(); end

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

  # Returns a [Hash](https://ruby-doc.org/core-2.6.3/Hash.html) using named
  # capture.
  # 
  # A key of the hash is a name of the named captures. A value of the hash
  # is a string of last successful capture of corresponding group.
  # 
  # ```ruby
  # m = /(?.)(?.)/.match("01")
  # m.named_captures #=> {"a" => "0", "b" => "1"}
  # 
  # m = /(?.)(?.)?/.match("0")
  # m.named_captures #=> {"a" => "0", "b" => nil}
  # 
  # m = /(?.)(?.)/.match("01")
  # m.named_captures #=> {"a" => "1"}
  # 
  # m = /(?x)|(?y)/.match("x")
  # m.named_captures #=> {"a" => "x"}
  # ```
  sig {returns(T::Hash[String, T.nilable(String)])}
  def named_captures(); end

  sig {returns(T::Array[String])}
  def names(); end

  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Integer])
  end
  def offset(n); end

  # Returns the portion of the original string after the current match.
  # Equivalent to the special variable `$'` .
  # 
  # ```ruby
  # m = /(.)(.)(\d+)(\d)/.match("THX1138: The Movie")
  # m.post_match   #=> ": The Movie"
  # ```
  sig {returns(String)}
  def post_match(); end

  # Returns the portion of the original string before the current match.
  # Equivalent to the special variable `` $` `` .
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

  # Returns a frozen copy of the string passed in to `match` .
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
  # Because `to_a` is called when expanding `*` *variable* , there’s a
  # useful assignment shortcut for extracting matched fields. This is
  # slightly slower than accessing the fields directly (as an intermediate
  # array is generated).
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

  sig do
    params(
        indexes: Integer,
    )
    .returns(T::Array[String])
  end
  def values_at(*indexes); end
end
