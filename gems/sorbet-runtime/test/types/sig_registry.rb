# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::SigRegistryTest < Critic::Unit::UnitTest
  SIG_BLOCK_RUN_ORDER_FIXTURE = "#{__dir__}/fixtures/sig_block_run_order.rb"
  SIG_IN_TRAP_CONTEXT_FIXTURE = "#{__dir__}/fixtures/sig_in_trap_context.rb"

  # Run in a subprocess so the assertion over T::Utils.run_all_sig_blocks
  # (which drains every pending sig block in the process) is hermetic.
  it 'run_all_sig_blocks runs pending sig blocks in global declaration order' do
    result, status = Open3.capture2("ruby", SIG_BLOCK_RUN_ORDER_FIXTURE)
    assert(status.success?, "ruby failed (exit #{status.exitstatus})")
    assert_equal("aa_m1,bb_m2,aa_m3,aa_singleton_m4,aa_m5\n", result)
  end

  it 'declaring and first-calling sig methods works inside a signal trap handler' do
    skip("no USR1 signal on this platform") unless Signal.list.key?("USR1")
    result, status = Open3.capture2("ruby", SIG_IN_TRAP_CONTEXT_FIXTURE)
    assert(status.success?, "ruby failed (exit #{status.exitstatus})")
    assert_equal("first-call-in-trap: 42\ndeclare-in-trap: 7\n", result)
  end
end
