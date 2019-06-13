# typed: __STDLIB_INTERNAL

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
