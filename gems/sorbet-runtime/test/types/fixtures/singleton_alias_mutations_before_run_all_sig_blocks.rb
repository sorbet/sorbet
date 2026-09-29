# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

class SingletonAliasMutationsBeforeRunAllSigBlocks
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def self.original
    :original
  end

  class << self
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
end

T::Utils.run_all_sig_blocks

klass = SingletonAliasMutationsBeforeRunAllSigBlocks
raise 'unchanged singleton alias stopped working' unless klass.unchanged == :original
raise 'redefined singleton alias was overwritten' unless klass.redefined == :redefined
raise 'removed singleton alias was restored' if klass.respond_to?(:removed)

puts 'PASS'
