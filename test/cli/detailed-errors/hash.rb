# typed: strict
extend T::Sig

sig { params(string_hash: T.nilable(T::Hash[String, Integer])).void }
def takes_string_hash(string_hash)
end

sig { params(symbol_hash: T.nilable(T::Hash[Symbol, Integer])).void }
def takes_symbol_hash(symbol_hash)
  takes_string_hash(symbol_hash)
end
