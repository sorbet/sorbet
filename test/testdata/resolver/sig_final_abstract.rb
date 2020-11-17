# typed: true

class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig(:final) {abstract.void}
  def final_abstract_lul; end

  sig(:final) {void}
  def just_final; end
end

class Child < Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {override.void}
  def final_abstract_lul; end # ... two errors?
# ^^^^^^^^^^^^^^^^^^^^^^ error: `Parent#final_abstract_lul` was declared as final and cannot be overridden by `Child#final_abstract_lul`
# ^^^^^^^^^^^^^^^^^^^^^^ error: Method overrides a final method `Parent#final_abstract_lul`

  sig {void}
  def just_final; end
# ^^^^^^^^^^^^^^ error: `Parent#just_final` was declared as final and cannot be overridden by `Child#just_final`
end
