# typed: strict

class Consumer::Use
  extend T::Sig

  sig {void}
  def self.run
    Producer::Private
    Producer::Private::Nested
    Producer::Private::Value
    puts 1
  end
end
