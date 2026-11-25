# typed: true

class Bar
    # ^ def: Bar 1 not-def-of-self
  Cnst = 1
  # ^ def: Cnst
end

module Foo
  Alias = Bar
  # ^ def: Alias
  #       ^ go-to-def-special: Bar

  module Other
         # ^ def: Other 1 not-def-of-self
    Alias = Bar
    # ^ def: OtherAlias_Inner
    #       ^ go-to-def-special: Bar
  end

  OtherAlias = Other
  # ^ def: OtherAlias
  #             ^ go-to-def-special: Other

  DeepAlias = OtherAlias::Alias
  # ^ def: DeepAlias
  #           ^ usage: OtherAlias
  #                       ^ usage: OtherAlias_Inner
end

module Foo
  Alias::Cnst
  # ^ usage: Alias
  #      ^ usage: Cnst

  DeepAlias::Cnst
  # ^ usage: DeepAlias
  #           ^ usage: Cnst
end
