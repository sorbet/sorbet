# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

class AliasRedefinedBeforeRunAllSigBlocks
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def original
    :original
  end
  alias copy original

  # rubocop:disable Lint/DuplicateMethods -- The redefinition is the behavior under test.
  def copy
    :replacement
  end
  # rubocop:enable Lint/DuplicateMethods
end

unless AliasRedefinedBeforeRunAllSigBlocks.new.copy == :replacement
  raise 'redefined alias did not initially have its replacement implementation'
end

T::Utils.run_all_sig_blocks

unless AliasRedefinedBeforeRunAllSigBlocks.new.copy == :replacement
  raise 'run_all_sig_blocks overwrote the redefined alias with the original implementation'
end

puts 'PASS'
