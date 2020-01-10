# typed: true

module Outer
  DefinedInOuter = nil

  module Middle
    DefinedInMiddle = nil

    module Inner
      DefinedInInner = nil

      DefinedIn # error: Unable to resolve
      #        ^ completion: DefinedInInner, DefinedInMiddle, DefinedInOuter

      Integer::DefinedIn # error: Unable to resolve
      #                 ^ completion: (nothing)
    end

    DefinedIn # error: Unable to resolve
    #        ^ completion: DefinedInMiddle, DefinedInOuter
  end

  DefinedIn # error: Unable to resolve
  #        ^ completion: DefinedInOuter
end

p DefinedIn # error: Unable to resolve
#          ^ completion: (nothing)
