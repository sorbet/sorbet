# typed: strict

extend T::Sig

sig { params(stuff: T::Array).void }
def bare_array(stuff); end

sig { params(stuff: T::Hash).void }
def bare_hash(stuff); end

sig { params(stuff: T::Set).void }
def bare_set(stuff); end

sig { params(stuff: T::Range).void }
def bare_range(stuff); end

sig { params(stuff: T::Enumerable).void }
def bare_enumerable(stuff); end

sig { params(stuff: T::Enumerator).void }
def bare_enumerator(stuff); end
