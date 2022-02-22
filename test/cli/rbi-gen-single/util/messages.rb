# typed: true

module Util

  class Messages
    extend T::Sig

    sig {params(msg: String).void}
    def self.say(msg)
      puts msg
    end

  end

end
