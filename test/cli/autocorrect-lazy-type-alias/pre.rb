MyAlias = T.type_alias(T.any(Integer, String))

HashWithCurly = T.type_alias({foo: String, bar: Integer})

BareHash = T.type_alias(foo: String, bar: Integer)
