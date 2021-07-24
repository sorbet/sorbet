# frozen_string_literal: true
# typed: true
# compiled: true

class ForeignClass
end

class AdvancedODM
  include T::Props

  prop :foreign_lazy, String, foreign: -> {ForeignClass}
  prop :foreign_proc, String, foreign: proc {ForeignClass}
  prop :foreign_invalid, String, foreign: proc { :not_a_type }
end

advanced_odm = AdvancedODM.new
p advanced_odm.method(:foreign_lazy_).parameters
