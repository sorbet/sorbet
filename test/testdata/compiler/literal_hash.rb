# frozen_string_literal: true
# typed: true
# compiled: true

# string key, other values
s = { "foo" => 1, "bar" => 2.0, "baz" => true, "quux" => false, "wat" => nil, "how" => "unknown", :do => "magic" }
# other kinds of key
i = { 1 => "one" }
f = { 2.0 => "two" }
t = { true => "true" }
a = { false => "false" }
n = { nil => "nil" }
y = { :symbol => "symbol" }
puts s, i, f, t, a, n, y
