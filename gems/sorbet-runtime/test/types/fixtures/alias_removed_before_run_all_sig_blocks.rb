# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

class AliasRemovedBeforeRunAllSigBlocks
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def original
    :original
  end
  alias copy original

  remove_method :copy
end

T::Utils.run_all_sig_blocks

if AliasRemovedBeforeRunAllSigBlocks.method_defined?(:copy)
  raise 'run_all_sig_blocks restored the removed alias'
end

puts 'PASS'
