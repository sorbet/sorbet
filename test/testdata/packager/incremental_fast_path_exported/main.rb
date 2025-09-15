# typed: strict
# spacer for exclude-from-file-update

module Root
  class Main
    T.reveal_type(Lib::MyStaticField) # error: `Integer`

    T.reveal_type(Lib::MyPrivateStaticField)
    #             ^^^^^^^^^^^^^^^^^^^^^^^^^ error: resolves but is not exported
  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Integer`
  end
end
