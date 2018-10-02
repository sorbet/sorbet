# The minimal portion of our Chalk::ODM support

class Chalk::ODM::Mutator
end

class Chalk::ODM::Mutator::Private::HashMutator
  extend T::Generic

  K = type_member
  V = type_member

  Sorbet.sig {params(key: K, value: V).void}
  def []=(key, value)
  end
end

class Chalk::ODM::Mutator::Private::ArrayMutator
  extend T::Generic

  Elem = type_member

  Sorbet.sig {params(value: Elem).void}
  def <<(value)
  end

  Sorbet.sig {params(key: Integer, value: Elem).void}
  def []=(key, value)
  end
end

class Chalk::ODM::Mutator::Private::DocumentMutator
end
