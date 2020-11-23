# typed: true
# frozen_string_literal: true

class Foo
  attr_reader :m1
#              ^ apply-rename: [A] newName: n1 invalid: true
  attr_writer :m2
#              ^ apply-rename: [B] newName: n2 invalid: true
  attr_accessor :m3
#                ^ apply-rename: [C] newName: n3 invalid: true
end

f = Foo.new
f.m1
# ^ apply-rename: [D] newName: n1 invalid: true
