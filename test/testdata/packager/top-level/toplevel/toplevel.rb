# frozen_string_literal: true
# typed: strict
# enable-packager: true

module ::Toplevel
  extend T::Sig

  sig {void}
  def self.hello
    puts "hello"
  end
end
