# typed: true
# frozen_string_literal: true
# compiled: true

extend T::Helpers

class Parent
  extend T::Sig
  sig { returns(Integer).checked(:compiled) }
  def foo
    1
  end
end
class Child < Parent
  extend T::Sig
  sig { returns(String).checked(:compiled) }
  def foo
    "a"
  end
end

begin
  T::Utils.run_all_sig_blocks
rescue => e
  puts e.message.include?("Incompatible return type in signature for override of method `foo`")
end
