# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

class MultipleAliasMutationsBeforeRunAllSigBlocks
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def original
    :original
  end

  alias unchanged original
  alias redefined original
  alias removed original

  # rubocop:disable Lint/DuplicateMethods -- The redefinition is the behavior under test.
  def redefined
    :redefined
  end
  # rubocop:enable Lint/DuplicateMethods

  remove_method :removed
end

T::Utils.run_all_sig_blocks

instance = MultipleAliasMutationsBeforeRunAllSigBlocks.new
raise 'unchanged alias stopped working' unless instance.unchanged == :original
raise 'redefined alias was overwritten' unless instance.redefined == :redefined
raise 'removed alias was restored' if instance.respond_to?(:removed)

puts 'PASS'
