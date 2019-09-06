# typed: true
class Foo
  A = T.type_alias(Foo) # error: Type aliases are not allowed in generic classes
  A = type_member # error: Redefining constant `Foo::A`
    # ^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(Foo)`
end
