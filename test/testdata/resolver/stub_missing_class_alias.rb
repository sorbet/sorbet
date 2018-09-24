# frozen_string_literal: true
# typed: true

class O::B
  class J # this is NOT the J you're looking for :-)
  end

  Document = O::D # error: Unable to resolve constant
  Doc1 = 1

  extend T::Helpers


  sig(arg: Document::J, # error: Unable to resolve constant `J`
   arg1: Doc1::J).void  # error: Unable to resolve constant `J`
  def self.foo(arg, arg1); end
end
