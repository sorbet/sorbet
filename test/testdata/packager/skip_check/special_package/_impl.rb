# frozen_string_literal: true
# typed: strict

module SpecialPackage
  # acceptable, because `Library::Constant` is exported
  Library::Constant
  # acceptable, because `SpecialPackage` is opted in to bypass typical checks
  Library::UnexportedConstant
end
