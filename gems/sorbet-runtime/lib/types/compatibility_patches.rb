# typed: ignore

require_relative 'private/methods/_methods'

# Work around an interaction bug with sorbet-runtime and rspec-mocks,
# which occurs when using *_any_instance_of and and_call_original.
#
# When a sig is defined, sorbet-runtime will replace the sigged method
# with a wrapper that, upon first invocation, re-wraps the method with a faster
# implementation.
#
# When expect_any_instance_of is used, rspec stores a reference to the first wrapper,
# to be restored later.
#
# The first wrapper is invoked as part of the test and sorbet-runtime replaces
# the method definition with the second wrapper.
#
# But when mocks are cleaned up, rspec restores back to the first wrapper.
# Upon subsequent invocations, the first wrapper is called, and sorbet-runtime
# throws a runtime error, since this is an unexpected state.
#
# We work around this by forcing re-wrapping before rspec stores a reference
# to the method.
if defined? ::RSpec::Mocks::AnyInstance
  module T
    module CompatibilityPatches
      module RecorderExtensions
        def observe!(method_name)
          method = @klass.instance_method(method_name.to_sym)
          T::Private::Methods.maybe_run_sig_block_for_method(method)
          super(method_name)
        end
      end
      ::RSpec::Mocks::AnyInstance::Recorder.prepend(RecorderExtensions)
    end
  end
end
