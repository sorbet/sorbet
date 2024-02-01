# typed: strict
# selective-apply-code-action: refactor.extract
# assert-no-code-action: refactor.extract
#
# No code actions should be available for the props

class TestDSLBuilder
  dsl_required :example1, String
  #                         | apply-code-action: [A] Move method to a new module
end
