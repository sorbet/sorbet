# typed: true

# Note: completion and hover share logic for showing documentation.
# See test/testdata/lsp/completion/constants_all_kinds.rb for a test to look at.

# Docs for AAA
class AAA; end
#     ^ hover: Docs for AAA

# Docs for BBB
class BBB
  #   ^ hover: Docs for BBB
  extend T::Generic

  # Docs for XXX
  XXX = nil
  # ^ hover: Docs for XXX

  # Docs for CCC
  CCC = AAA # Arguably we should show CCC, but this is ~fine.
  # ^ hover: Docs for AAA

  # Docs for EEE
  EEE = type_member
  # ^ hover: Docs for EEE
end
