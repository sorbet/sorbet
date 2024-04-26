# typed: true

class Opus::Command
  extend T::Sig
  def self.call(*args, **kwargs, &blk)
  end
end

class MyCommand < Opus::Command
  sig { params(x: Integer, y: Integer).void }
  def call(x, y)
    # ^ def: MyCommand.call
  end
end

MyCommand.call(1, 2)
#         ^ usage: MyCommand.call
MyCommand.call(3, 4)
#         ^ usage: MyCommand.call
