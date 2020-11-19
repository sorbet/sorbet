# typed: false
extend T::Sig

class Parent
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.void}
  def becomes_private; end

  sig {abstract.void}
  private def becomes_public; end

  sig {abstract.void}
  def stays_public; end

  sig {abstract.void}
  private def stays_private; end
end

class Child < Parent
  abstract!

  private :becomes_private # error: Can't narrow visibility of `abstract` method `Parent#becomes_private`

  # Ok, because Liskov substitution principle still holds
  public :becomes_public
  public :stays_public
  private :stays_private
end
