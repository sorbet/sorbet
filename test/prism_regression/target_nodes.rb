# typed: false

##### Target nodes used in multi-assignments are tested separately in `test/prism_regression/assign_to_*.rb`

##### Test target nodes as the rescued value

begin
rescue => @ivar
end

begin
rescue => @@cvar
end

begin
rescue => $gvar
end

begin
rescue => self.call_target1
end

begin
rescue => self::call_target2
end

begin
rescue => ConstantTarget
end

begin
rescue => ::ConstantPathTarget
end

begin
rescue => Nested::ConstantPathTarget
end

begin
rescue => ::Nested::ConstantPathTarget
end

begin
rescue => a[123]
end

##### Test target nodes as a `for` loop variable

for @ivar in []
end

for @@cvar in []
end

for $gvar in []
end

for self.call_target1 in []
end

for self::call_target2 in []
end

for ConstantTarget in []
end

for ::ConstantPathTarget in []
end

for Nested::ConstantPathTarget in []
end

for ::Nested::ConstantPathTarget in []
end

for a[123] in []
end
