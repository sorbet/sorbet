# frozen_string_literal: true
# typed: strict

class AAA::AClass
  BBB
# ^^^ error: No import provides `BBB`

  CCC
# ^^^ error: No import provides `CCC`

  C
# ^ error: Unable to resolve constant `C`
end
