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
