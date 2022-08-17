module Opus::Types::Test::DuplicateSigEvalSandbox
  class DefineMethodWithSig
    extend T::Sig

    Alias = T.type_alias {T.nilable(T.class_of(Opus::Types::Test::DuplicateSigEvalSandbox::CallsMethodUponLoading))}

    sig {params(arg: Alias).void}
    def self.duplex(arg); end

    sig {void}
    def self.foo; end
  end
end
