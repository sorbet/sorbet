# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

class AliasRedirectedBeforeRunAllSigBlocks
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def original
    :original
  end

  def other
    :other
  end

  # rubocop:disable Lint/DuplicateMethods -- Redirecting the alias is the behavior under test.
  alias copy original
  alias copy other
  # rubocop:enable Lint/DuplicateMethods
end

T::Utils.run_all_sig_blocks

unless AliasRedirectedBeforeRunAllSigBlocks.new.copy == :other
  raise 'run_all_sig_blocks redirected the alias back to the signed method'
end

puts 'PASS'
