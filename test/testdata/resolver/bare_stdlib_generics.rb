# typed: strict

extend T::Sig

sig { params(stuff: T::Array).void } # error: Malformed type declaration. Generic class without type arguments `T::Array`
def bare_array(stuff); end

sig { params(stuff: T::Hash).void } # error: Malformed type declaration. Generic class without type arguments `T::Hash`
def bare_hash(stuff); end

sig { params(stuff: T::Set).void } # error: Malformed type declaration. Generic class without type arguments `T::Set`
def bare_set(stuff); end

sig { params(stuff: T::Range).void } # error: Malformed type declaration. Generic class without type arguments `T::Range`
def bare_range(stuff); end

sig { params(stuff: T::Enumerable).void } # error: Malformed type declaration. Generic class without type arguments `T::Enumerable`
def bare_enumerable(stuff); end

sig { params(stuff: T::Enumerator).void } # error: Malformed type declaration. Generic class without type arguments `T::Enumerator`
def bare_enumerator(stuff); end
