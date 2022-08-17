# typed: strict

# No error (by analogy with how we handle sig'd method redefinitions?)
X = T.let(0, Integer)
X = T.let('', String)

Alias = Integer
#       ^^^^^^^ error: Expected `T.class_of(String)` but found `T.class_of(Integer)` for field
Alias = String

BadAlias = 1 + 1
#          ^^^^^ error: Expected `T.class_of(Sorbet::Private::Static::StubModule)` but found `Integer` for field
BadAlias = DoesNotExist
#          ^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`
