# frozen_string_literal: true
# typed: true

class ::Sorbet::Private::Static::PackageSpec
  extend T::Sig

  # Define the things referenced in __package.rb
  def self.some_method(x); end

  def self.method_call_arg; end

  ConstantArg = 10
end
