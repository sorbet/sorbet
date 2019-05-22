# The minimal portion of our Chalk::ODM support
# typed: __STDLIB_INTERNAL

class Chalk::ODM::Mutator
end

class Chalk::ODM::Mutator::Private::HashMutator
  extend T::Generic

  K = type_member
  V = type_member

  sig {params(key: K, value: V).void}
  def []=(key, value)
  end
end

class Chalk::ODM::Mutator::Private::ArrayMutator
  extend T::Generic

  Elem = type_member

  sig {params(value: Elem).void}
  def <<(value)
  end

  sig {params(key: Integer, value: Elem).void}
  def []=(key, value)
  end
end

class Chalk::ODM::Mutator::Private::DocumentMutator
end

class Chalk::ODM::Document
end

class Opus::DB::Model::Mixins::Encryptable::EncryptedValue < Chalk::ODM::Document
  sig {params(options: Hash).returns(Opus::DB::Model::Mixins::Encryptable::EncryptedValue)}
  def initialize(options)
  end
end
