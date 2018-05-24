# typed: true
class Opus::Command; end

class MyCommand < Opus::Command
  sig(x: Integer).returns(String)
  def call(x)
    x.to_s
  end
end

T.assert_type!(MyCommand.call(7), String)

class OtherCommand < ::Opus::Command
  sig(x: String).returns(Integer)
  def call(x)
    Integer(x)
  end
end

T.assert_type!(OtherCommand.call("8"), Integer)

class NotACommand < Llamas::Opus::Command # error: Unable to resolve constant
  sig(x: String).returns(Integer)
  def call(x)
    Integer(x)
  end
end

NotACommand.call # error: Method `call` does not exist

class CallNoSig < Opus::Command
  def call(x)
    Integer(x)
  end
end

CallNoSig.call # error: Method `call` does not exist
