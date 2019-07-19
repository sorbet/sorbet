# typed: true

T.assert_type!(catch(1) { throw 1 }, BasicObject)
T.assert_type!(catch(1) { throw 1, 'test' }, BasicObject)
T.assert_type!(catch {|obj_A| throw(obj_A, 'test')}, BasicObject)
T.assert_type!(catch {1}, BasicObject)

T.assert_type!(caller, T::Array[String])
T.assert_type!(caller(10), T.nilable(T::Array[String]))

x = loop {break 1}
T.assert_type!(x, Integer)
if x
  puts x + 1
end

define_singleton_method(:foo) { puts '' }
define_singleton_method('foo') { puts '' }

y = loop do
end
puts y # error: This code is unreachable
