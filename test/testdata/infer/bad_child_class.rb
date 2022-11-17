# typed: true
# disable-fast-path: true

# found by fuzzer: https://github.com/sorbet/sorbet/issues/1080

class PreChild <Parent # error: Type `K` declared by parent `Parent` must be re-declared in `PreChild`
  extend T::Generic
  ::V = type_member

end

class Parent
  extend T::Sig
  extend T::Generic
  K = type_member
  sig {returns(K)}
  def foo; T.unsafe(nil); end
  puts PreChild.new.foo() # this line previously caused a failed ENFORCE in LambdaParam::_instantiate
end
