# typed: strict
# disable-parser-comparison: true
# See <https://github.com/sorbet/sorbet/issues/10383>.
# (This test happens to use syntax that Prism parses differently from Sorbet.)
extend T::Sig

MyAlias = T.type_alias { String }.checked(:never)
TestsAlias = T.type_alias { Integer }.checked(:tests)
AlwaysAlias = T.type_alias { Symbol }.checked(:always)

sig { params(x: MyAlias).returns(MyAlias) }
def foo(x)
  x
end

sig { params(x: TestsAlias).returns(TestsAlias) }
def bar(x)
  x
end

sig { params(x: AlwaysAlias).returns(AlwaysAlias) }
def baz(x)
  x
end

BadAlias = T.type_alias { DoesNotExist }.checked(:never) # error: Unable to resolve right hand side of type alias `BadAlias`
#                         ^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`

class HasGeneric
  extend T::Generic
  Elem = type_member
  MyElem = T.type_alias { Elem }.checked(:tests) # error: Defining a `type_alias` to a generic `type_member` is not allowed
end

sig { returns(MyAlias) }
def wrong_return
  0 # error: Expected `String` but found `Integer(0)` for method result type
end

ForgotAliasBlock = T.type_alias.checked(:tests)
#                  ^^^^^^^^^^^^ error: No block given
