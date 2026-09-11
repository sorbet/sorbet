# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

class AliasOfPreviousMethodDefinition
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def value
    :original
  end
  alias raw_value value

  sig { returns(Symbol).checked(:never) }
  # rubocop:disable Lint/DuplicateMethods -- Replacing the signed method is the behavior under test.
  def value
    raw_value
  end
  # rubocop:enable Lint/DuplicateMethods
end

T::Utils.run_all_sig_blocks

unless AliasOfPreviousMethodDefinition.new.value == :original
  raise 'alias was associated with the replacement method and became recursive'
end

puts 'PASS'
