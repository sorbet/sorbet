# frozen_string_literal: true
# typed: strict

module NormalPackage
  # acceptable, because `Library::Constant` is exported
  Library::Constant
  # unacceptable!
  Library::UnexportedConstant # error: `Library::UnexportedConstant` resolves but is not exported
end
