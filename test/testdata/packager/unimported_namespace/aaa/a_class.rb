# frozen_string_literal: true
# typed: strict

class AAA::AClass
  BBB
# ^^^ error: `BBB` is not imported

  CCC
# ^^^ error: `CCC` is not imported

  C
# ^ error: Unable to resolve constant `C`
end
