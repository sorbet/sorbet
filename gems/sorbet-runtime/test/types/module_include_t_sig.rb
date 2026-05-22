# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::ModuleIncludeTSig < Critic::Unit::UnitTest
  MODULE_INCLUDE_T_SIG_FIXTURE = "#{__dir__}/fixtures/module_include_t_sig.rb"

  it 'sigs work on all classes/modules when Module includes T::Sig' do
    result, status = Open3.capture2("ruby", MODULE_INCLUDE_T_SIG_FIXTURE)
    assert(status.success?, "ruby failed (exit #{status.exitstatus})")
    assert_equal(<<~EXPECTED, result)
      called A#foo
      Expected type Integer, got type String with value "nope"
      called A.bar
      Expected type Symbol, got type Float with value 0.0
      called M#baz
      Expected type Integer, got type String with value "bad"
    EXPECTED
  end
end
