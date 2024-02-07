# typed: true

# Note: If we ever change Sorbet's payload to make StdioInputMethod the
# canonical version, let's change this test to mention IRB::InputMethod, the
# old superclass, so that we don't have to delete the test.
class IRB::RelineInputMethod < ::IRB::StdioInputMethod
end
