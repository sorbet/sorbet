# frozen_string_literal: true
# typed: true
# compiled: true
a = ''
a.instance_eval do
  @a = :a
  @b = :b
  @c = :c
end
100000.times do
  a = Marshal.load(Marshal.dump(a))
end
#p(a.instance_eval { @a == :a && @b == :b && @c == :c })
