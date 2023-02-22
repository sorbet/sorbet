# typed: true

module M
  extend T::Sig

  sig {returns(T.attached_class)}
  #            ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in a singleton class method context, and not in modules
  def instance_method
  end

  sig {returns(T.attached_class)}
  #            ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in a singleton class method context, and not in modules
  def self.class_method
  end
end
