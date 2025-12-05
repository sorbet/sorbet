# typed: true

class ForeignClass
end

class AdvancedODM
    include T::Props
    prop :foreign_lazy, String, foreign: -> {ForeignClass}
    prop :foreign_proc, String, foreign: proc {ForeignClass}
    prop :foreign_proc, String, foreign: Kernel.proc {ForeignClass}
    prop :foreign_proc, String, foreign: ::Kernel.proc {ForeignClass}
    prop :foreign_invalid, String, foreign: proc { :not_a_type }
    prop :foreign_proc, String, foreign: Kernel.proc { :not_a_type }
    prop :foreign_proc, String, foreign: ::Kernel.proc { :not_a_type }
end
