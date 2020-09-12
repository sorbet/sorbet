# frozen_string_literal: true
# typed: true
# compiled: true

# At one point, the compiler optimized final methods by calling the
# implementation function directly. However, when final methods are called
# without going through the vm, they won't have a corresponding frame on the
# ruby stack. The result of this is that if they allocate any locals, they will
# overwrite the state of the parent frame, and the locals will be shifted by the
# number of locals the called final method uses.
#
# This test exposes this problem by showing that because the implementation
# function mutates the state on the top of the ruby stack, the caller will have
# its locals corrupted.

class Foo
  extend T::Sig

  sig(:final) {void}
  def self.test
    y = 'overwritten local'
    1.times do
      y
    end
  end
end

x = 'initial local'
1.times do
  x
end

p x # initial local
Foo.test
p x # overwritten local
