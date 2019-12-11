# typed: true

class NoArgs
  extend T::Sig

  sig {returns(T.experimental_attached_class)}
  def self.make
    T.reveal_type(self.new) # error: Revealed type: `T.attached_class (of NoArgs)`
  end

  sig {returns(T.experimental_attached_class)}
  def self.make_implicit_new
    T.reveal_type(new) # error: Revealed type: `T.attached_class (of NoArgs)`
  end

end

class PosArgs
  extend T::Sig

  sig {params(x: Integer, y: String).void}
  def initialize(x, y); end

  sig {returns(T.experimental_attached_class)}
  def self.make
    T.reveal_type(self.new(10, "foo")) # error: Revealed type: `T.attached_class (of PosArgs)`

    # Not matching initialize
    T.reveal_type(self.new())
                # ^^^^^^^^^^ error: Not enough arguments provided
  # ^^^^^^^^^^^^^^^^^^^^^^^^^ error: Revealed type: `T.attached_class (of PosArgs)`
  end
end

class NamedArgs
  extend T::Sig

  sig {params(x: Integer, y: String).void}
  def initialize(x:, y:); end

  sig {returns(T.experimental_attached_class)}
  def self.make
    T.reveal_type(self.new(x: 10, y: "foo")) # error: Revealed type: `T.attached_class (of NamedArgs)`

    # Not matching initialize
    T.reveal_type(self.new(x: 10))
                # ^^^^^^^^^^^^^^^ error: Missing required keyword argument `y`
  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Revealed type: `T.attached_class (of NamedArgs)`
  end
end

class BlockArg
  extend T::Sig

  sig {params(blk: T.proc.params(x: Integer).void).void}
  def initialize(&blk); end

  sig {returns(T.experimental_attached_class)}
  def self.make
    T.reveal_type(self.new {|x| x + 1}) # error: Revealed type: `T.attached_class (of BlockArg)`

    T.reveal_type(self.new)
                # ^^^^^^^^ error: `initialize` requires a block parameter
  # ^^^^^^^^^^^^^^^^^^^^^^^ error: Revealed type: `T.attached_class (of BlockArg)`
  end
end

# In this case there is an instance method called new, and the
# `Magic.<self-new>` intrinsic should handle it.
class InstNew
  extend T::Sig

  sig {params(x: Integer).returns(String)}
  def new(x)
    x.to_s
  end

  def test
    T.reveal_type(self.new(10)) # error: Revealed type: `String`
  end
end

# In this case there is an instance method called new that takes a block
# parameter, and the `Magic.<self-new>` intrinsic should handle it.
class InstNewBlock
  extend T::Sig

  sig {params(blk: T.proc.params(x: Integer).void).returns(String)}
  def new(&blk)
    "foo"
  end

  def test
    T.reveal_type(self.new {|x| x + 1}) # error: Revealed type: `String`
  end
end
