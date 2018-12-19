# typed: true

T.assert_type!(catch(1) { throw 1 }, BasicObject)
T.assert_type!(catch(1) { throw 1, 'test' }, BasicObject)
T.assert_type!(catch {|obj_A| throw(obj_A, 'test')}, BasicObject)
T.assert_type!(catch {1}, BasicObject)

T.assert_type!(caller, T::Array[String])
T.assert_type!(caller(10), T.nilable(T::Array[String]))

# make sure we don't regress and mark `loop` as returning `nil`
x = loop {break 1}
if x
  puts x + 1
else
  puts x
end
