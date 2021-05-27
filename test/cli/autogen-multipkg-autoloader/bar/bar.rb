# typed: strict

class Project::Bar::BarClass
  extend T::Sig
  sig {params(value: Integer).void}
  def initialize(value)
    @value = T.let(value, Integer)
  end
end
