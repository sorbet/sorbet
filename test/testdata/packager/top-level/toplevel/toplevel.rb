# frozen_string_literal: true
# typed: strict

module ::Toplevel # error: was previously defined
  extend T::Sig

  sig {void}
  def self.hello
    puts "hello"
  end
end
