module Opus::Types::Test::DuplicateSigEvalSandbox
  class CallsMethodUponLoading
    Opus::Types::Test::DuplicateSigEvalSandbox::DefineMethodWithSig.foo
    Opus::Types::Test::DuplicateSigEvalSandbox::DefineMethodWithSig.duplex(self)
  end
end
