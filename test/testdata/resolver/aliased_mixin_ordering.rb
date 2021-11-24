# typed: strict

module Stringy
  extend T::Sig

  sig {returns(String)}
  def f
    "1"
  end
end

module Inty
  extend T::Sig

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
T.reveal_type(MixesDirect.new.f) # error: Revealed type: `Integer`
T.reveal_type(MixesDirectSwapped.new.f) # error: Revealed type: `String`
