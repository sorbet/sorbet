# typed: true

# Only way to observe that a method is private right now is via completion.

def foo_method_root; end

foo_method_ # error: does not exist
#          ^ completion: foo_method_root

class A
  def foo_method_inner_public; end

  private def foo_method_inner_private; end
end

A.new.foo_method_ # error: does not exist
#                ^ completion: foo_method_inner_public, foo_method_root
