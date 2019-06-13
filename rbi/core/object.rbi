# typed: __STDLIB_INTERNAL

class Object < BasicObject
  include Kernel

  sig {returns(Integer)}
  def object_id(); end
end

class Class < Module
  sig {returns(T.untyped)}
  def allocate(); end

  # Sorbet hijacks Class#new to re-use the sig from MyClass#initialize when creating new instances of a class.
  # This method must be here so that all calls to MyClass.new aren't forced to take 0 arguments.
  sig {params(args: T.untyped).returns(T.untyped)}
  def new(*args); end

  sig do
    params(
        arg0: Class,
    )
    .returns(T.untyped)
  end
  def inherited(arg0); end

  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def instance_methods(arg0=T.unsafe(nil)); end

  sig {returns(T.nilable(String))}
  def name(); end

  sig {returns(T.nilable(Class))}
  sig {returns(Class)}
  def superclass(); end

  sig {void}
  sig do
    params(
        superclass: Class,
    )
    .void
  end
  sig do
    params(
        blk: T.proc.params(arg0: Class).returns(BasicObject),
    )
    .void
  end
  sig do
    params(
        superclass: Class,
        blk: T.proc.params(arg0: Class).returns(BasicObject),
    )
    .void
  end
  def initialize(superclass=Object, &blk); end
end

class Data < Object
end

class NilClass < Object
  sig do
    params(
        obj: BasicObject,
    )
    .returns(FalseClass)
  end
  def &(obj); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ^(obj); end

  sig {returns(Rational)}
  def rationalize(); end

  sig {returns([])}
  def to_a(); end

  sig {returns(Complex)}
  def to_c(); end

  sig {returns(Float)}
  def to_f(); end

  sig {returns(T::Hash[T.untyped, T.untyped])}
  def to_h(); end

  sig {returns(Rational)}
  def to_r(); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def |(obj); end

  sig {returns(TrueClass)}
  def nil?; end
end

class TrueClass
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(TrueClass)}
  def |(obj)
  end
  sig {returns(FalseClass)}
  def !
  end
end

class FalseClass
  sig {params(obj: BasicObject).returns(FalseClass)}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def |(obj)
  end
  sig {returns(TrueClass)}
  def !
  end
end

::ARGF = T.let(T.unsafe(nil), Object)
::ARGV = T.let(T.unsafe(nil), Array)
::CROSS_COMPILING = T.let(T.unsafe(nil), NilClass)
::FALSE = T.let(T.unsafe(nil), FalseClass)
::NIL = T.let(T.unsafe(nil), NilClass)
::RUBY_COPYRIGHT = T.let(T.unsafe(nil), String)
::RUBY_DESCRIPTION = T.let(T.unsafe(nil), String)
::RUBY_ENGINE = T.let(T.unsafe(nil), String)
::RUBY_ENGINE_VERSION = T.let(T.unsafe(nil), String)
::RUBY_PATCHLEVEL = T.let(T.unsafe(nil), Integer)
::RUBY_PLATFORM = T.let(T.unsafe(nil), String)
::RUBY_RELEASE_DATE = T.let(T.unsafe(nil), String)
::RUBY_REVISION = T.let(T.unsafe(nil), Integer)
::RUBY_VERSION = T.let(T.unsafe(nil), String)
::STDERR = T.let(T.unsafe(nil), IO)
::STDIN = T.let(T.unsafe(nil), IO)
::STDOUT = T.let(T.unsafe(nil), IO)
::TOPLEVEL_BINDING = T.let(T.unsafe(nil), Binding)
::TRUE = T.let(T.unsafe(nil), TrueClass)
