# typed: true
# spacer for exclude-from-file-update

  class Child < Parent
    MyElem = type_member

    sig {override.params(x: MyElem).void}
    def example(x)
    end
  end
