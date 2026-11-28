# typed: strict

# Not the configured registry file, so this registration is rejected.
T::Descendants.register(A::Parent, {B::OtherChild => "b/other_child.rb"})
