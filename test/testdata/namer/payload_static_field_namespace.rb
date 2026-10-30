# typed: true

module CSV::DEFAULT_OPTIONS::Foo
#      ^^^^^^^^^^^^^^^^^^^^ error: Redefining constant `DEFAULT_OPTIONS` as a class or module
  DoesNotExist
# ^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`
end

class Sorbet::Private::Static::IOLike::A
#     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Redefining constant `IOLike` as a class or module
end

class Queue::A
#     ^^^^^ error: Redefining constant `Queue` as a class or module
end

class Bundler::EndpointSpecification::Elem::A
#     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Redefining constant `Elem` as a class or module
end

# Rejected definitions must not modify the canonical alias target.
  Thread::Queue::A
# ^^^^^^^^^^^^^^^^ error: Unable to resolve constant `A`

# Payload aliases have already resolved before project naming begins.
  SizedQueue::AliasConstant = 2
# ^^^^^^^^^^ error: Redefining constant `SizedQueue` as a class or module
  class ConditionVariable
# ^^^^^^^^^^^^^^^^^^^^^^^ error: Redefining constant `ConditionVariable` as a class or module
  def alias_method; end
end
  class ConditionVariable
# ^^^^^^^^^^^^^^^^^^^^^^^ error: Redefining constant `ConditionVariable` as a class or module
  def another_alias_method; end
end

# Rejected definitions must not modify the canonical alias targets.
  Thread::SizedQueue::AliasConstant
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `AliasConstant`
Thread::ConditionVariable.new.alias_method
#                             ^^^^^^^^^^^^ error: Method `alias_method` does not exist

# Unlike the type-template alias above, this is a type member itself.
class Array::Elem::A
#     ^^^^^^^^^^^ error: Redefining constant `Elem` as a class or module
end
