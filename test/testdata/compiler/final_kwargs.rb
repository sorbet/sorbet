# typed: true
# compiled: true
# frozen_string_literal: true


class Foo
  extend T::Sig

  sig(:final) {params(x: Integer, y: String).void}
  def self.kwargs(x:, y:)
    puts "x = #{x}, y = #{y}"
  end
end

Foo.kwargs(x: 10, y: 'hello')
