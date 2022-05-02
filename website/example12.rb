# typed: true
class Opus::WorkflowEngine::ActivityInterface
  extend T::Sig
  extend T::Helpers
  
  sig{returns(T.attached_class)}
  def self.remote
    remove_name_for_registry = activity_name

    remote_handle = T.unsafe(nil).do_smth_with_activity_name(remove_name_for_registry)

    remote_handle
  end

  abstract!

  sig{abstract.returns(String)}
  def self.activity_name; end
end

class MyActivity < Opus::WorkflowEngine::ActivityInterface
  extend T::Sig
  extend T::Helpers

  abstract!

  # Docs
  sig{abstract.params(a: String).returns(Integer)}
  def foo(a); end

  def self.activity_name
    "My name that's visible on the wire"
  end
end

class MyActivityImplementation < MyActivity

  extend T::Sig

  sig{override.params(a: String).returns(Integer)}
  def foo(a)
    1
  end
end

# use site

MyActivity.remote.foo("1")
