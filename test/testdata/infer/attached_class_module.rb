# typed: true

module M
  extend T::Sig

  sig {returns(T.attached_class)}
  #            ^^^^^^^^^^^^^^^^ error: `M` must declare `has_attached_class!` before module instance methods can use `T.attached_class
  def instance_method
  end

  sig {returns(T.attached_class)}
  #            ^^^^^^^^^^^^^^^^ error: `T.attached_class` cannot be used in singleton methods on modules, because modules cannot be instantiated
  def self.class_method
  end
end
