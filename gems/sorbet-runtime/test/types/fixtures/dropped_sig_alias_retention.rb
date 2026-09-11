# typed: true
# frozen_string_literal: true
require 'weakref'
require_relative '../../../lib/sorbet-runtime'

parent = Class.new do
  extend T::Sig

  sig { returns(Symbol).checked(:never) }
  def original
    :original
  end
end

def make_subclass_ref(parent)
  subclass = Class.new(parent) do
    alias_method :copy, :original
  end
  WeakRef.new(subclass)
end

subclass_ref = make_subclass_ref(parent)

parent.define_method(:original) { :replacement }

10.times do
  GC.start(full_mark: true, immediate_sweep: true)
  break unless subclass_ref.weakref_alive?
end

raise 'discarded alias tracking retained the subclass' if subclass_ref.weakref_alive?

puts 'PASS'
