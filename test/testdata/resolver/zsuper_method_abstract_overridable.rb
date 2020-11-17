# typed: false
extend T::Sig

class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  def method_no_sig; end

  sig {void}
  def method_with_sig; end

  sig {abstract.void}
  def method_with_abstract_sig; end

  sig {overridable.void}
  def method_with_overridable_sig; end
end

class Child < Parent
  abstract!

  # No error, to be backwards compatible with Ruby
  # (even though by Liskov substitution we would want one)
  private :method_no_sig

  # No error, to make Sorbet adoption easier
  # (even though by Liskov substitution we would want one)
  private :method_with_sig

  private :method_with_abstract_sig # error: Can't narrow visibility of abstract method `method_with_abstract_sig`

  private :method_with_overridable_sig # error: Can't narrow visibility of overridable method `method_with_overridable_sig`
end
