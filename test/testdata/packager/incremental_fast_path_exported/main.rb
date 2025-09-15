# typed: strict

module Root
  class Main
    p(Lib::MyStaticField)

    p(Lib::MyPrivateStaticField) # error: resolves but is not exported
  end
end
