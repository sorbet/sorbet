# typed: strict

class Consumer::Use
  extend T::Sig

  sig {void}
  def self.run
    Producer::Private # error: `Producer::Private` resolves but is not exported
    Producer::Private::Nested # error: `Producer::Private::Nested` resolves but is not exported
    Producer::Private::Value # error: `Producer::Private::Value` resolves but is not exported
    puts 1
  end
end
