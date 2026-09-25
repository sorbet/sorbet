# typed: strict

module Stringy
  extend T::Sig

  X = "1"

  sig {returns(String)}
  def f
    "1"
  end
end

module Inty
  extend T::Sig

  X = 1

  sig {returns(Integer)}
  def f
    1
  end
end

class MixesAlias
  extend T::Sig

  StringyAlias = Stringy

  include StringyAlias
  include Inty

  T.reveal_type(X) # error: Revealed type: `Integer`
end

class MixesAliasReverse
  extend T::Sig

  StringyAlias = Stringy

  include Inty
  include StringyAlias

  T.reveal_type(X) # error: Revealed type: `String`
end

class MixesDirect
  extend T::Sig

  include Stringy
  include Inty
end

class MixesDirectSwapped
  extend T::Sig

  include Inty
  include Stringy
end

T.reveal_type(MixesAlias.new.f) # error: Revealed type: `Integer`
T.reveal_type(MixesAliasReverse.new.f) # error: Revealed type: `String`
T.reveal_type(MixesDirect.new.f) # error: Revealed type: `Integer`
T.reveal_type(MixesDirectSwapped.new.f) # error: Revealed type: `String`
