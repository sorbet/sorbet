# typed: true

class A
end

res = A.new.instance_exec do |arg0|
  T.reveal_type(arg0) # error: `NilClass`
  1
end
T.reveal_type(res) # error: `Integer`

res = A.new.instance_exec(5) do |arg0|
  T.reveal_type(arg0) # error: `T.untyped`
  ''
end
T.reveal_type(res) # error: `String`
