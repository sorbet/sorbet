# typed: true

class Base
  extend T::Sig

  sig { params(block: T.proc.bind(T.attached_class).void).void }
  def self.before_create(&block); end

  before_create do
    T.reveal_type(self) # error: Revealed type: `Base`
  end
end

Base.before_create do
  T.reveal_type(self) # error: Revealed type: `Base`
end

class Model < Base
  before_create do
    T.reveal_type(self) # error: Revealed type: `Model`

    does_exist
    does_not_exist # error: Method `does_not_exist` does not exist on `Model`
  end

  def does_exist; end
end

Model.before_create do
  T.reveal_type(self) # error: Revealed type: `Model`

  does_exist
  does_not_exist # error: Method `does_not_exist` does not exist on `Model`
end

class BadBinds
  extend T::Sig

  sig {params(it: T.proc.bind(T.attached_class).void).void}
            # ^^ error: Using `bind` is not permitted here
  def self.non_block_argument(it); end

  sig {returns(T.proc.bind(T.attached_class).void)}
             # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Using `bind` is not permitted here
  def self.attached_class_return
    T.unsafe(nil)
  end
end

module DoesNotHaveAttachedClass
  extend T::Sig

  sig { params(blk: T.proc.bind(T.attached_class).void).returns(T.attached_class) }
  #                             ^^^^^^^^^^^^^^^^ error: `DoesNotHaveAttachedClass` must declare `has_attached_class!` before module instance methods can use `T.attached_class`
  #                                                             ^^^^^^^^^^^^^^^^ error: `DoesNotHaveAttachedClass` must declare `has_attached_class!` before module instance methods can use `T.attached_class`

  def with_bind(&blk)
  end

  def example
    with_bind {
      T.reveal_type(self) # error: `DoesNotHaveAttachedClass`
    }
  end
end

class ExtendsDoesNotHave
  extend DoesNotHaveAttachedClass
  def self.example
    with_bind {
      T.reveal_type(self) # error: `T.class_of(ExtendsDoesNotHave)`
    }
  end
end

module HasAttachedClass
  extend T::Sig, T::Generic
  abstract!

  has_attached_class!(:out)

  sig { params(blk: T.proc.bind(T.attached_class).void).returns(T.attached_class) }
  def with_bind(&blk)
    Kernel.raise
  end

  def example
    with_bind {
      T.reveal_type(self) # error: `T.anything`
    }
  end
end

class ExtendsHas
  extend HasAttachedClass
  def self.example
    with_bind {
      T.reveal_type(self) # error: `ExtendsHas`
    }
  end
end
