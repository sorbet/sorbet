# typed: true

module Parent
end

module Child
  include Parent
end

module A
  extend T::Sig
  sig {params(parent: Parent).void}
  def example(parent)
    if parent.is_a?(Child)
    end
  end
end
