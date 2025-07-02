# typed: true
class Opus::Command
  extend T::Sig
end

class MyCommand < Opus::Command
  sig {params(x: Integer).returns(String)}
  def call(x)
    x.to_s
  end
end

T.assert_type!(MyCommand.call(7), String)

MyCommand.new.call(12) # error: Non-private call to private method `call` on `MyCommand`

class OtherCommand < ::Opus::Command
  sig {params(x: String).returns(Integer)}
  def call(x)
    Integer(x)
  end
end

T.assert_type!(OtherCommand.call("8"), Integer)

class NotACommand < Llamas::Opus::Command # error: Unable to resolve constant
  extend T::Sig

  sig {params(x: String).returns(Integer)}
  def call(x)
    Integer(x) # error: Method `Integer` does not exist on `NotACommand`
  end
end

NotACommand.call # error: Method `call` does not exist

class CallNoSig < Opus::Command
  def call(x)
    Integer(x)
  end
end

CallNoSig.call # error: Method `call` does not exist

module AbstractMixin
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { abstract.params(z: Integer).returns(Integer) }
  def foo(z); end
end

class ConcreteCommand < Opus::Command
  include AbstractMixin

  # This secretly becomes a private method and so should error.
  sig { override.params(z: Integer).returns(Integer) }
  def foo(z); z; end # error: Method `foo` is private in `ConcreteCommand` but not in `AbstractMixin`
end

module AbstractMixin
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { abstract.params(z: Integer).returns(Integer) }
  def foo(z); end
end

class ConcreteCommand < Opus::Command
  include AbstractMixin

  # This secretly becomes a private method and so should error.
  sig { override.params(z: Integer).returns(Integer) }
  def foo(z); z; end # error: Method `foo` is private in `ConcreteCommand` but not in `AbstractMixin`
end

# This isn't a command and should still work
class NotACommand
  include AbstractMixin

  sig { override.params(z: Integer).returns(Integer) }
  def foo(z); 0; end
end

class CommandWithCallAndMixin < Opus::Command
  include AbstractMixin

  sig {params(x: String).returns(Integer)}
  def call(x); Integer(x); end

  sig { override.params(z: Integer).returns(Integer) }
  def foo(z); 0; end # error: Method `foo` is private in `CommandWithCallAndMixin` but not in `AbstractMixin`
end
