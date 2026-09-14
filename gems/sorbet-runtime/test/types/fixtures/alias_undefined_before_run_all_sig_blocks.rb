# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

class AliasUndefinedBeforeRunAllSigBlocks
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def original
    :original
  end
  alias copy original

  undef_method :copy
end

T::Utils.run_all_sig_blocks

if AliasUndefinedBeforeRunAllSigBlocks.method_defined?(:copy)
  raise 'run_all_sig_blocks restored the undefined alias'
end

puts 'PASS'
