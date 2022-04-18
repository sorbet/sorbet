# frozen_string_literal: true
# typed: strict

class AAA::AClass
  BBB
# ^^^ error: Unable to resolve constant `BBB`

  C
# ^ error: Unable to resolve constant `C`
end
