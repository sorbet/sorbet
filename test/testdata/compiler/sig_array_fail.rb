# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig
  sig {params(xs: T::Array[Integer]).void}
  def self.foo(xs)
    puts 'No sig failure!'
  end
end

A.foo(T.unsafe(['']))
