# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

module SignedAliasSource
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def original
    :original
  end
end

class IncludedAliasMutation
  include SignedAliasSource
  alias copy original
  remove_method :copy
end

class ExtendedAliasMutation
  extend SignedAliasSource

  class << self
    alias copy original
    remove_method :copy
  end
end

T::Utils.run_all_sig_blocks

raise 'removed included-method alias was restored' if IncludedAliasMutation.method_defined?(:copy)
raise 'removed extended-method alias was restored' if ExtendedAliasMutation.respond_to?(:copy)

puts 'PASS'
