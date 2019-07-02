# typed: true
# disable-fast-path: true

# found by fuzzer: https://github.com/sorbet/sorbet/issues/1080

class PreChild <Parent # error: Type `K` declared by parent `Parent` must be re-declared in `PreChild`
 ::V = type_member
end

class Parent
  extend T::Sig
  K = type_member
  sig {returns(K)}
  def foo; T.unsafe(nil); end
  puts PreChild.new.foo() # this line previously caused a failed ENFORCE in LambdaParam::_instantiate
end
