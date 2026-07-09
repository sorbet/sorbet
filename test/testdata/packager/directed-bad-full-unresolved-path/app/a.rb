# typed: strict
# enable-packager: true
# enable-package-directed: true

module App
  class A
    extend T::Sig

    ATLP = Lib::AliasToLongProto

    sig { returns(ATLP::DoesNotExist2) }
    #             ^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant
    def example = raise
  end
end
