# typed: true

class Parent
  private_class_method :new

  def self.make
    new
  end

  def self.make_explicit
    self.new
  end
end

class Child < Parent; end

class PublicChild < Parent
  public_class_method :new
end

class OwnNew < Parent
  def self.new
    :own
  end
end

Parent.new # error: Non-private call to private method `new`
Child.new # error: Non-private call to private method `new`
PublicChild.new
OwnNew.new

Parent.make
Child.make
PublicChild.make
OwnNew.make

module MixinNew
  def new
    :from_mixin
  end
end

class MixinParent
  private_class_method :new
end

class MixinChild < MixinParent
  extend MixinNew
end

class MixinPrivateChild < MixinParent
  extend MixinNew
  private_class_method :new
end

MixinChild.new
MixinPrivateChild.new # error: Non-private call to private method `new`

class RealParent
  def self.new
    :parent
  end
  private_class_method :new
end

class RealChild < RealParent; end

class RealPublicChild < RealParent
  public_class_method :new
end

RealParent.new # error: Non-private call to private method `new`
RealChild.new # error: Non-private call to private method `new`
RealPublicChild.new

class WithInitialize
  def initialize(x); end
  private_class_method :new
end

class WithInitializeChild < WithInitialize; end

WithInitialize.new(0) # error: Non-private call to private method `new`
WithInitializeChild.new(0) # error: Non-private call to private method `new`

class PublicInitializeChild < WithInitialize
  public_class_method :new
end

PublicInitializeChild.new # error: Not enough arguments provided for method `WithInitialize#initialize`
PublicInitializeChild.new(0)

class EigenParent
  class << self
    private :new
  end
end

class EigenChild < EigenParent; end

class EigenPublicChild < EigenParent
  class << self
    public :new
  end
end

EigenParent.new # error: Non-private call to private method `new`
EigenChild.new # error: Non-private call to private method `new`
EigenPublicChild.new
