# typed: true

module Util

  class Messages
    extend T::Sig

    sig {params(msg: String).void}
    def self.say(msg)
      puts msg
    end

    sig do
      type_parameters(:T)
        .params(msg: GenericMessage[T.type_parameter(:T)])
        .void
    end
    def self.print_message(msg)
      print msg
    end

  end

end
