# typed: strict
# enable-experimental-rbs-comments: true

hash1 = {
  a: 1, #: as String
  b: 2, #: as String
  c: {
    d: 3 #: as String
  },
}
T.reveal_type(hash1) # error: Revealed type: `{a: String, b: String, c: {d: String}} (shape of T::Hash[T.untyped, T.untyped])`

{
  a: 1,
  b: 2,
} #: Hash[Symbol, Integer]

hash2 = {} #: Hash[Symbol, Integer]
T.reveal_type(hash2) # error: Revealed type: `T::Hash[Symbol, Integer]`

hash3 = {
  **hash2,
} #: Hash[Symbol, Integer]
T.reveal_type(hash3) # error: Revealed type: `T::Hash[Symbol, Integer]`

hash4 = {
  **hash2 #: as Hash[Symbol, String]
}
T.reveal_type(hash4) # error: Revealed type: `T::Hash[Symbol, String]`

hash5 = {
  **hash2, #: as Hash[Symbol, String]
}
T.reveal_type(hash5) # error: Revealed type: `T::Hash[Symbol, String]`
