# typed: strict
class Module; include T::Sig; end

module ThisOneExists
  sig {returns(Integer)}
  def this_one_exists; 0; end
end

module A
  extend ThisOneExists

  class << self
    sig {void}
    def bar
      x = super # error: Method `bar` does not exist on ancestors
      T.reveal_type(x) # error: `T.untyped`
    end

    sig {void}
    def this_one_exists
      x = super
      T.reveal_type(x) # error: `Integer`
    end
  end
end

module ModuleSelfDotMethods
  extend ThisOneExists

  sig {void}
  def self.bar
    x = super # error: Method `bar` does not exist on ancestors
    T.reveal_type(x) # error: `T.untyped`
  end

  sig {void}
  def self.this_one_exists
    x = super
    T.reveal_type(x) # error: `Integer`
  end
end
