# typed: true

module Mixin
  extend T::Sig

  sig { params(block: T.proc.bind(T.self_type).void).void }
  def foo(&block); end
end

class IncludesMixin
  include Mixin

  def does_exist; end

  def bar
    foo do
      T.reveal_type(self) # error: Revealed type: `T.self_type (of IncludesMixin)`

      does_exist
      does_not_exist # error: Method `does_not_exist` does not exist on `IncludesMixin`
    end
  end
end

IncludesMixin.new.foo do
  T.reveal_type(self) # error: Revealed type: `IncludesMixin`

  does_exist
  does_not_exist # error: Method `does_not_exist` does not exist on `IncludesMixin`
end

class Base
  extend T::Sig

  sig { params(block: T.proc.bind(T.self_type).void).void }
  def self.foo(&block); end

  sig { params(block: T.proc.bind(T.self_type).void).void }
  def instance_foo(&block); end
end

class Model < Base
  def self.does_exist; end
  def does_exist; end

  foo do
    T.reveal_type(self) # error: Revealed type: `T.self_type (of T.class_of(Model))`

    does_exist
    does_not_exist # error: Method `does_not_exist` does not exist on `T.class_of(Model)`
  end

  def bar
    instance_foo do
      T.reveal_type(self) # error: Revealed type: `T.self_type (of Model)`

      does_exist
      does_not_exist # error: Method `does_not_exist` does not exist on `Model`
    end
  end
end

Model.foo do
  T.reveal_type(self) # error: Revealed type: `T.class_of(Model)`

  does_exist
  does_not_exist # error: Method `does_not_exist` does not exist on `T.class_of(Model)`
end

Model.new.instance_foo do
  T.reveal_type(self) # error: Revealed type: `Model`

  does_exist
  does_not_exist # error: Method `does_not_exist` does not exist on `Model`
end
