# typed: strong

module Opus
  class Command
  end
end

class MyCommand < Opus::Command
  extend T::Sig

  sig {void}
  def call
    nil
  end
end
