# typed: strict

class GoodUsages
  extend T::Sig
  # basic usages
  delegate :foo, :bar, to: :thing
  delegate :baz, to: :@ivar
  delegate :ball, to: :thing, private: true, allow_nil: true
  # with prefix
  delegate :foo, :bar, prefix: 'string', to: :thing
  delegate :foo, :bar, prefix: :symbol, to: :thing
  delegate :foo, :bar, to: :true, prefix: true

  sig {void}
  def usages
    foo {}
    bar(1, 3, 4, 5)
    baz
    ball(thing: 0) {}
    string_foo
    string_bar
    symbol_foo {}
    symbol_bar(1, 2) {}
    true_foo
    true_bar
  end
end

class WorksWithoutExtendingTSig
  delegate :foo, :bar, to: :thing
end

class IgnoredUsages
  local = 0
  not_delegate :thing, to: :nop # error: Method `not_delegate` does not exist
  delegate # error: Method `delegate` does not exist
  delegate :thing # error: Method `delegate` does not exist
  delegate to: :thing # error: Method `delegate` does not exist
  delegate :thing, prefix: :thing # error: Method `delegate` does not exist
  delegate :foo, to: :thing, prefix: local # error: Method `delegate` does not exist
  delegate :foo, local => :thing # error: Method `delegate` does not exist
  delegate 234, to: :good # error: Method `delegate` does not exist
  delegate :thing => :thing # error: Method `delegate` does not exist
  # these raise at runtime
  delegate :foo, :bar, to: :@hi, prefix: true # error: Method `delegate` does not exist
  delegate :foo, :bar, to: '', prefix: true # error: Method `delegate` does not exist
end

class EnumerableUsage
  extend T::Generic

  include Enumerable

  Elem = type_member

  delegate :each, to: :foo
end
