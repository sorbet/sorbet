# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

class ParentWithAlias
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def original
    :original
  end
  alias copy original
end

class ChildWithRemovedAlias < ParentWithAlias
  alias copy original
  remove_method :copy
end

unless ChildWithRemovedAlias.instance_method(:copy).owner == ParentWithAlias
  raise 'removing the child alias did not reveal the inherited alias'
end

T::Utils.run_all_sig_blocks

unless ChildWithRemovedAlias.instance_method(:copy).owner == ParentWithAlias
  raise 'run_all_sig_blocks recreated the removed alias on the child class'
end

ParentWithAlias.define_method(:copy) { :replacement }

unless ChildWithRemovedAlias.new.copy == :replacement
  raise 'the child no longer inherits a later replacement of the parent method'
end

puts 'PASS'
