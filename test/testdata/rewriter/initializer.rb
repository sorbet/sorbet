# typed: true
class Foo
  extend T::Sig

  sig { params(x: Integer).void }
  def initialize(x)
    @x = x  # this will get a synthetic `T.let`
    @y = x + 1  # This should not, because the RHS is not a simple local
  end

  sig { void }
  def foo
    T.reveal_type(@x) # error: Revealed type: `Integer`
    T.reveal_type(@y) # error: Revealed type: `T.untyped`
  end
end

class Bar
  extend T::Sig

  # we should find `params` correctly here
  sig { overridable.params(x: Integer).overridable.void }
  def initialize(x)
    @x = x
  end

  sig { void }
  def foo
    T.reveal_type(@x) # error: Revealed type: `Integer`
  end
end

class Baz
  extend T::Sig

  # nobody should ever write this, but just in case, we should make
  # sure not to include type_parameter types in a generated `T.let`
  sig { type_parameters(:U).params(x: T.type_parameter(:U)).void }
  def initialize(x)
    @x = x
  end

  sig { void }
  def foo
    T.reveal_type(@x) # error: Revealed type: `T.untyped`
  end
end

class Literals
  extend T::Sig

  sig {void}
  def initialize
    @int = 0
    @str = 'str'
    @sym = :sym
    @float = 0.0
    @nil = nil
    @true = true
    @false = false
    @cls = Object
    @obj = Object.new
  end

  sig {void}
  def foo
    T.reveal_type(@int) # error: Revealed type: `Integer`
    T.reveal_type(@str) # error: Revealed type: `String`
    T.reveal_type(@sym) # error: Revealed type: `Symbol`
    T.reveal_type(@float) # error: Revealed type: `Float`
    T.reveal_type(@nil) # error: Revealed type: `NilClass`
    T.reveal_type(@true) # error: Revealed type: `TrueClass`
    T.reveal_type(@false) # error: Revealed type: `FalseClass`
    T.reveal_type(@cls) # error: Revealed type: `T.untyped`
    T.reveal_type(@obj) # error: Revealed type: `T.untyped`
  end
end

# Some miscellaneous weirder cases
class InterspersedExprs
  extend T::Sig

  sig { params(x: Integer).void }
  def initialize(x)
    ["hello", "world"].each do |w|
      puts w
    end
    @x = x
    puts "goodbye"
    @y = :y
  end

  sig { void }
  def foo
    T.reveal_type(@x) # error: Revealed type: `Integer`
    T.reveal_type(@y) # error: Revealed type: `Symbol`
  end
end

# Some less straightforward situations with arguments
class DifferentArgs
  extend T::Sig

  sig { params(x: Integer, y: T.nilable(String), z: T.any(T::Array[T::Boolean], T.proc.void)).void }
  def initialize(x, y, z)
    @a = y  # it works out-of-order
    r = x
    @b = r  # it does NOT work with a variable that's not explicitly a param
    @c = y  # a single param can be used with multiple instance variables
    [1, 2].each do
      @d1 = z  # it does NOT work within blocks
      @d2 = :z
    end
    if true
      @e1 = z  # it does NOT work within other control flow
      @e2 = :z
    end

    @f = z  # it does work with more sophisticated types

    l = 0
    @g = l # it does NOT work with a variable derived from a literal
    @h = 1 + 1 # it does NOT work with a literal expression
  end

  sig { void }
  def foo
    T.reveal_type(@a)  # error: Revealed type: `T.nilable(String)`
    T.reveal_type(@b)  # error: Revealed type: `T.untyped`
    T.reveal_type(@c)  # error: Revealed type: `T.nilable(String)
    T.reveal_type(@d1)  # error: Revealed type: `T.untyped`
    T.reveal_type(@d2)  # error: Revealed type: `T.untyped`
    T.reveal_type(@e1)  # error: Revealed type: `T.untyped`
    T.reveal_type(@e2)  # error: Revealed type: `T.untyped`
    T.reveal_type(@f)  # error: Revealed type: `T.any(T::Array[T::Boolean], T.proc.void)`
    T.reveal_type(@g)  # error: Revealed type: `T.untyped`
    T.reveal_type(@h)  # error: Revealed type: `T.untyped`
  end
end

class ClassVar
  extend T::Sig
  sig { params(x: Integer).void }
  def initialize(x)
    @@a = x  # it does NOT currently work with class variables
  end

  sig { void }
  def foo
    T.reveal_type(@@a)  # error: Revealed type: `T.untyped`
  end
end

class NestedConstructor
  extend T::Sig

  sig {void}
  def self.foo
    sig {params(x: Integer).void}
    def initialize(x)
      # it does not work if the constructor is not declared as a
      # top-level method of the class (i.e. if it is nested inside
      # something else) due to limitations about how we associate sigs
      @x = x
    end
  end

  def foo
    T.reveal_type(@x)  # error: Revealed type: `T.untyped`
  end
end

class Checked
  extend T::Sig

  sig {params(x: Integer).void.checked(:never)}
  def initialize(x)
    @x = x
  end

  sig {void}
  def foo
    T.reveal_type(@x) # error: Revealed type: `Integer`
  end
end

class OnFailure
  extend T::Sig

  sig {params(x: Integer).void.on_failure(:soft, notify: 'sorbet')}
  def initialize(x)
    @x = x
  end

  sig {void}
  def foo
    T.reveal_type(@x) # error: Revealed type: `Integer`
  end
end

class BadSigReturn
  extend T::Sig

  sig {params(x: Integer).returns(Integer)} # error: The initialize method should always return void
  def initialize(x)
    @x = x
  end
end

class BadSigReturnNotLastStatement
  extend T::Sig

  sig {params(x: Integer).returns(Integer).on_failure(:soft, notify: 'sorbet')} # error: The initialize method should always return void
  def initialize(x)
    @x = x
  end
end
