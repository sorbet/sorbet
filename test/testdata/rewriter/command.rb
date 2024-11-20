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
