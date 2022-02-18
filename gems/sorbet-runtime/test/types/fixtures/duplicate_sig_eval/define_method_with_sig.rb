module Opus::Types::Test::DuplicateSigEvalSandbox
  class DefineMethodWithSig
    extend T::Sig

    sig {params(arg: T.nilable(T.class_of(Opus::Types::Test::DuplicateSigEvalSandbox::CallsMethodUponLoading))).void}
    def self.duplex(arg); end

    sig {void}
    def self.foo; end
  end
end
