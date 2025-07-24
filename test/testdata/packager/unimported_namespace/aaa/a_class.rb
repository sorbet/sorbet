# frozen_string_literal: true
# typed: strict

class AAA::AClass
  BBB
# ^^^ error: `BBB` resolves but its package is not imported

  CCC
# ^^^ error: `CCC` resolves but its package is not imported

  C
# ^ error: Unable to resolve constant `C`
end
