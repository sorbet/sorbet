# frozen_string_literal: true
# compiled: true
# typed: true

module Main
  extend T::Sig

  sig {params(x: T::Array[Integer]).void}
  def self.example(x)
    x.freeze
  end
end

Main.example([1,2,3])
