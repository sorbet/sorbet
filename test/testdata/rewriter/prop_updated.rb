# typed: strict

module Chalk::ODM
  DeprecatedNumeric = Numeric
end


class A < Chalk::ODM::Document
  extend T::Sig

  sig {params(opts: T::Hash[T.untyped, T.untyped]).void}
  def self.updated_prop(opts={})
    opts[:extra] = opts.fetch(:extra, {}).merge(DEPRECATED_dynamic_prop: true)
    self.prop(:updated, T.nilable(Chalk::ODM::DeprecatedNumeric), opts)
  end

  updated_prop
end

updated_at = T.reveal_type(A.new.updated) # error: Revealed type: `Numeric`
