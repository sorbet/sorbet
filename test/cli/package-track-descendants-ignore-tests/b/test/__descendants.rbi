# typed: strict

# Registering a descendant from a test file is still allowed.
T::Descendants.register(A::Parent, {Test::B::RegisteredTestChild => "b/test/registered_test_child.rb"})
