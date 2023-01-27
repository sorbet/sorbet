# frozen_string_literal: true
# typed: strict

class Package::A
  extend T::Sig

  sig {returns(String)}
  def self.a
    Dep::Exports::ExportedClass::Val
    "asdf"
  end

  sig {returns(String)}
  def self.b
    "foo"
  end
end
