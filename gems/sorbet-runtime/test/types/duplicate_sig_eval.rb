# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::DuplicateSigEvalTest < Critic::Unit::UnitTest
  before do
    module Opus::Types::Test::DuplicateSigEvalSandbox; end
  end

  after do
    Opus::Types::Test.send(:remove_const, :DuplicateSigEvalSandbox)
  end

  it "allows duplicate signature evaluation" do
    module Opus::Types::Test::DuplicateSigEvalSandbox
      autoload :CallsMethodUponLoading, "#{__dir__}/fixtures/duplicate_sig_eval/calls_method_upon_loading"
      autoload :DefineMethodWithSig, "#{__dir__}/fixtures/duplicate_sig_eval/define_method_with_sig"
    end

    # Shouldn't raise error
    Opus::Types::Test::DuplicateSigEvalSandbox::DefineMethodWithSig.duplex(nil)

    sigs = T::Private::Methods.instance_variable_get(:@signatures_by_method)
    object_id = Opus::Types::Test::DuplicateSigEvalSandbox::DefineMethodWithSig.singleton_class.object_id

    foo_sig = sigs["#{object_id}#foo"]
    duplex_sig = sigs["#{object_id}#duplex"]

    assert_equal(:always, foo_sig.check_level) # check_level would be :never if sig wasn't fully evaluated
    assert_equal(:always, duplex_sig.check_level)
  end
end
