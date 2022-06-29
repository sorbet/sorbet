# frozen_string_literal: true
# typed: strict

class MyRoot::A
  extend T::Sig
  sig {returns(Integer)}
  def run
    '' # error: Expected `Integer` but found `String("")` for method result type
  end
end
