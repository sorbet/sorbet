MyAlias = T.type_alias {T.any(Integer, String)}

HashWithCurly = T.type_alias {{foo: String, bar: Integer}}

BareHash = T.type_alias {{foo: String, bar: Integer}}

module Bar
  class Foo
    Multiline = T.type_alias do
      T.any(
        String,
        Integer
      )
    end

    MyType = T.type_alias do
      {
        foo: String,
        bar: T.nilable(Integer)
      }
    end
  end
end
