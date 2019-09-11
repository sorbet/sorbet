# frozen_string_literal: true
# typed: true

class O::B
  class J # this is NOT the J you're looking for :-)
  end

  Document = O::D # error: Unable to resolve constant
  Doc1 = 1

  extend T::Sig


  sig do
    params(
      arg0: Document::J, # error: Unable to resolve constant `J`
      arg1: Doc1::J      # error: Unable to resolve constant `J`
    )
    .void
  end
  def self.foo(arg0, arg1); end
end
