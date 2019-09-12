# typed: __STDLIB_INTERNAL

# [`Random`](https://docs.ruby-lang.org/en/2.6.0/Random.html) provides an
# interface to Ruby's pseudo-random number generator, or PRNG. The PRNG produces
# a deterministic sequence of bits which approximate true randomness. The
# sequence may be represented by integers, floats, or binary strings.
#
# The generator may be initialized with either a system-generated or
# user-supplied seed value by using
# [`Random.srand`](https://docs.ruby-lang.org/en/2.6.0/Random.html#method-c-srand).
#
# The class method
# [`Random.rand`](https://docs.ruby-lang.org/en/2.6.0/Random.html#method-c-rand)
# provides the base functionality of
# [`Kernel.rand`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-rand)
# along with better handling of floating point values. These are both interfaces
# to
# [`Random::DEFAULT`](https://docs.ruby-lang.org/en/2.6.0/Random.html#DEFAULT),
# the Ruby system PRNG.
#
# [`Random.new`](https://docs.ruby-lang.org/en/2.6.0/Random.html#method-c-new)
# will create a new PRNG with a state independent of
# [`Random::DEFAULT`](https://docs.ruby-lang.org/en/2.6.0/Random.html#DEFAULT),
# allowing multiple generators with different seed values or sequence positions
# to exist simultaneously.
# [`Random`](https://docs.ruby-lang.org/en/2.6.0/Random.html) objects can be
# marshaled, allowing sequences to be saved and resumed.
#
# PRNGs are currently implemented as a modified Mersenne Twister with a period
# of 2\*\*19937-1.
class Random < Object
  include Random::Formatter

  # The default Pseudorandom number generator. Used by class methods of
  # [`Random`](https://docs.ruby-lang.org/en/2.6.0/Random.html).
  DEFAULT = T.let(T.unsafe(nil), Random)

  # Returns true if the two generators have the same internal state, otherwise
  # false. Equivalent generators will return the same sequence of pseudo-random
  # numbers. Two generators will generally have the same state only if they were
  # initialized with the same seed
  #
  # ```ruby
  # Random.new == Random.new             # => false
  # Random.new(1234) == Random.new(1234) # => true
  # ```
  #
  # and have the same invocation history.
  #
  # ```ruby
  # prng1 = Random.new(1234)
  # prng2 = Random.new(1234)
  # prng1 == prng2 # => true
  #
  # prng1.rand     # => 0.1915194503788923
  # prng1 == prng2 # => false
  #
  # prng2.rand     # => 0.1915194503788923
  # prng1 == prng2 # => true
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Returns a random binary string containing `size` bytes.
  #
  # ```ruby
  # random_string = Random.new.bytes(10) # => "\xD7:R\xAB?\x83\xCE\xFAkO"
  # random_string.size                   # => 10
  # ```
  sig do
    params(
        size: Integer,
    )
    .returns(String)
  end
  def bytes(size); end

  sig do
    params(
        seed: Integer,
    )
    .void
  end
  def initialize(seed=T.unsafe(nil)); end

  # When `max` is an
  # [`Integer`](https://docs.ruby-lang.org/en/2.6.0/Integer.html), `rand`
  # returns a random integer greater than or equal to zero and less than `max`.
  # Unlike
  # [`Kernel.rand`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-rand),
  # when `max` is a negative integer or zero, `rand` raises an
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html).
  #
  # ```ruby
  # prng = Random.new
  # prng.rand(100)       # => 42
  # ```
  #
  # When `max` is a [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html),
  # `rand` returns a random floating point number between 0.0 and `max`,
  # including 0.0 and excluding `max`.
  #
  # ```ruby
  # prng.rand(1.5)       # => 1.4600282860034115
  # ```
  #
  # When `max` is a [`Range`](https://docs.ruby-lang.org/en/2.6.0/Range.html),
  # `rand` returns a random number where range.member?(number) == true.
  #
  # ```ruby
  # prng.rand(5..9)      # => one of [5, 6, 7, 8, 9]
  # prng.rand(5...9)     # => one of [5, 6, 7, 8]
  # prng.rand(5.0..9.0)  # => between 5.0 and 9.0, including 9.0
  # prng.rand(5.0...9.0) # => between 5.0 and 9.0, excluding 9.0
  # ```
  #
  # Both the beginning and ending values of the range must respond to subtract
  # (`-`) and add (`+`)methods, or rand will raise an
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html).
  sig do
    params(
        max: T.any(Integer, T::Range[Integer]),
    )
    .returns(Integer)
  end
  sig do
    params(
        max: T.any(Float, T::Range[Float]),
    )
    .returns(Float)
  end
  def rand(max=T.unsafe(nil)); end

  # Returns the seed value used to initialize the generator. This may be used to
  # initialize another generator with the same state at a later time, causing it
  # to produce the same sequence of numbers.
  #
  # ```ruby
  # prng1 = Random.new(1234)
  # prng1.seed       #=> 1234
  # prng1.rand(100)  #=> 47
  #
  # prng2 = Random.new(prng1.seed)
  # prng2.rand(100)  #=> 47
  # ```
  sig {returns(Integer)}
  def seed(); end

  # Returns an arbitrary seed value. This is used by
  # [`Random.new`](https://docs.ruby-lang.org/en/2.6.0/Random.html#method-c-new)
  # when no seed value is specified as an argument.
  #
  # ```ruby
  # Random.new_seed  #=> 115032730400174366788466674494640623225
  # ```
  sig {returns(Integer)}
  def self.new_seed(); end

  # Alias of Random::DEFAULT.rand.
  sig do
    params(
        max: Integer,
    )
    .returns(Numeric)
  end
  def self.rand(max=T.unsafe(nil)); end

  # Seeds the system pseudo-random number generator,
  # [`Random::DEFAULT`](https://docs.ruby-lang.org/en/2.6.0/Random.html#DEFAULT),
  # with `number`. The previous seed value is returned.
  #
  # If `number` is omitted, seeds the generator using a source of entropy
  # provided by the operating system, if available (/dev/urandom on Unix systems
  # or the RSA cryptographic provider on Windows), which is then combined with
  # the time, the process id, and a sequence number.
  #
  # srand may be used to ensure repeatable sequences of pseudo-random numbers
  # between different runs of the program. By setting the seed to a known value,
  # programs can be made deterministic during testing.
  #
  # ```ruby
  # srand 1234               # => 268519324636777531569100071560086917274
  # [ rand, rand ]           # => [0.1915194503788923, 0.6221087710398319]
  # [ rand(10), rand(1000) ] # => [4, 664]
  # srand 1234               # => 1234
  # [ rand, rand ]           # => [0.1915194503788923, 0.6221087710398319]
  # ```
  sig do
    params(
        number: Integer,
    )
    .returns(Numeric)
  end
  def self.srand(number=T.unsafe(nil)); end
end

# Format raw random number as
# [`Random`](https://docs.ruby-lang.org/en/2.6.0/Random.html) does
module Random::Formatter
  # SecureRandom.base64 generates a random base64 string.
  #
  # The argument *n* specifies the length, in bytes, of the random number to be
  # generated. The length of the result string is about 4/3 of *n*.
  #
  # If *n* is not specified or is nil, 16 is assumed. It may be larger in the
  # future.
  #
  # The result may contain A-Z, a-z, 0-9, "+", "/" and "=".
  #
  # ```ruby
  # require 'securerandom'
  #
  # SecureRandom.base64 #=> "/2BuBuLf3+WfSKyQbRcc/A=="
  # SecureRandom.base64 #=> "6BbW0pxO0YENxn38HMUbcQ=="
  # ```
  #
  # If a secure random number generator is not available, `NotImplementedError`
  # is raised.
  #
  # See RFC 3548 for the definition of base64.
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(String)
  end
  def base64(n=nil); end

  # SecureRandom.hex generates a random hexadecimal string.
  #
  # The argument *n* specifies the length, in bytes, of the random number to be
  # generated. The length of the resulting hexadecimal string is twice of *n*.
  #
  # If *n* is not specified or is nil, 16 is assumed. It may be larger in the
  # future.
  #
  # The result may contain 0-9 and a-f.
  #
  # ```ruby
  # require 'securerandom'
  #
  # SecureRandom.hex #=> "eb693ec8252cd630102fd0d0fb7c3485"
  # SecureRandom.hex #=> "91dc3bfb4de5b11d029d376634589b61"
  # ```
  #
  # If a secure random number generator is not available, `NotImplementedError`
  # is raised.
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(String)
  end
  def hex(n=nil); end

  # Generates formatted random number from raw random bytes. See
  # [`Random#rand`](https://docs.ruby-lang.org/en/2.6.0/Random.html#method-i-rand).
  sig {returns(Float)}
  sig do
    params(
      n: T.nilable(Float)
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(Numeric)
    )
    .returns(Numeric)
  end
  sig do
    params(
      n: T.nilable(T::Range[Float])
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(T::Range[Integer])
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(T::Range[Numeric])
    )
    .returns(Numeric)
  end
  def rand(n=nil); end

  # SecureRandom.random\_bytes generates a random binary string.
  #
  # The argument *n* specifies the length of the result string.
  #
  # If *n* is not specified or is nil, 16 is assumed. It may be larger in
  # future.
  #
  # The result may contain any byte: "x00" - "xff".
  #
  # ```ruby
  # require 'securerandom'
  #
  # SecureRandom.random_bytes #=> "\xD8\\\xE0\xF4\r\xB2\xFC*WM\xFF\x83\x18\xF45\xB6"
  # SecureRandom.random_bytes #=> "m\xDC\xFC/\a\x00Uf\xB2\xB2P\xBD\xFF6S\x97"
  # ```
  #
  # If a secure random number generator is not available, `NotImplementedError`
  # is raised.
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(String)
  end
  def random_bytes(n=nil); end

  # Generates formatted random number from raw random bytes. See
  # [`Random#rand`](https://docs.ruby-lang.org/en/2.6.0/Random.html#method-i-rand).
  sig {returns(Float)}
  sig do
    params(
      n: T.nilable(Float)
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(Numeric)
    )
    .returns(Numeric)
  end
  sig do
    params(
      n: T.nilable(T::Range[Float])
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(T::Range[Integer])
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(T::Range[Numeric])
    )
    .returns(Numeric)
  end
  def random_number(n=nil); end

  # SecureRandom.urlsafe\_base64 generates a random URL-safe base64 string.
  #
  # The argument *n* specifies the length, in bytes, of the random number to be
  # generated. The length of the result string is about 4/3 of *n*.
  #
  # If *n* is not specified or is nil, 16 is assumed. It may be larger in the
  # future.
  #
  # The boolean argument *padding* specifies the padding. If it is false or nil,
  # padding is not generated. Otherwise padding is generated. By default,
  # padding is not generated because "=" may be used as a URL delimiter.
  #
  # The result may contain A-Z, a-z, 0-9, "-" and "\_". "=" is also used if
  # *padding* is true.
  #
  # ```ruby
  # require 'securerandom'
  #
  # SecureRandom.urlsafe_base64 #=> "b4GOKm4pOYU_-BOXcrUGDg"
  # SecureRandom.urlsafe_base64 #=> "UZLdOkzop70Ddx-IJR0ABg"
  #
  # SecureRandom.urlsafe_base64(nil, true) #=> "i0XQ-7gglIsHGV2_BNPrdQ=="
  # SecureRandom.urlsafe_base64(nil, true) #=> "-M8rLhr7JEpJlqFGUMmOxg=="
  # ```
  #
  # If a secure random number generator is not available, `NotImplementedError`
  # is raised.
  #
  # See RFC 3548 for the definition of URL-safe base64.
  sig do
    params(
      n: T.nilable(Integer),
      padding: T::Boolean
    )
    .returns(String)
  end
  def urlsafe_base64(n=nil, padding=false); end

  # SecureRandom.uuid generates a random v4 UUID (Universally Unique
  # IDentifier).
  #
  # ```ruby
  # require 'securerandom'
  #
  # SecureRandom.uuid #=> "2d931510-d99f-494a-8c67-87feb05e1594"
  # SecureRandom.uuid #=> "bad85eb9-0713-4da7-8d36-07a8e4b00eab"
  # SecureRandom.uuid #=> "62936e70-1815-439b-bf89-8492855a7e6b"
  # ```
  #
  # The version 4 UUID is purely random (except the version). It doesn't contain
  # meaningful information such as MAC addresses, timestamps, etc.
  #
  # The result contains 122 random bits (15.25 random bytes).
  #
  # See RFC 4122 for details of UUID.
  sig {returns(String)}
  def uuid(); end
end
