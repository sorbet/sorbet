# typed: true
# enable-experimental-delegate-return-types: true
# disable-stress-incremental: true

class DelegateReturnTypeFoo
  extend T::Sig

  sig { returns(String) }
  def delegated_method
    T.unsafe(nil)
  end
end

class DelegateReturnTypeBar
  extend T::Sig

  delegate :delegated_method, to: :foo

  sig { returns(DelegateReturnTypeFoo) }
  def foo
    T.unsafe(nil)
  end
end

T.assert_type!(DelegateReturnTypeBar.new.delegated_method, String)

