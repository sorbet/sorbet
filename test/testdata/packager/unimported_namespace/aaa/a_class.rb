# frozen_string_literal: true
# typed: strict

class AAA::AClass
  BBB
# ^^^ error: No import provides `BBB`

  C
# ^ error: Unable to resolve constant `C`
end
