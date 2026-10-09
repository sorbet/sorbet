# typed: true
class Queue::AliasLspNested
#     ^^^^^ error: Redefining constant `Queue` as a class or module
  def foo
    1
  end
end
  Thread::Queue::AliasLspNested.new.foo
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `AliasLspNested`
