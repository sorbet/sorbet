# frozen_string_literal: true
# typed: strict

module NormalPackage
  # acceptable, because `Library::Constant` is exported
  Library::Constant
  # unacceptable!
  Library::UnexportedConstant
end
