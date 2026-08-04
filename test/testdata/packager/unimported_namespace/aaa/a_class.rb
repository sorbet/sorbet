# frozen_string_literal: true
# typed: strict

class AAA::AClass
  BBB
# ^^^ error: `BBB` is not imported

  CCC
# ^^^ error: Unable to resolve constant `CCC`

  C
# ^ error: Unable to resolve constant `C`
end
