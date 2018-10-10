# typed: true

T.assert_type!(Kernel.loop, Enumerator[T.untyped])

T.assert_type!(Kernel.loop {break}, NilClass)

# Kernel is implied
T.assert_type!(loop {break}, NilClass)

T.assert_type!(catch(1) { throw 1 }, BasicObject)
T.assert_type!(catch(1) { throw 1, 'test' }, BasicObject)
T.assert_type!(catch {|obj_A| throw(obj_A, 'test')}, BasicObject)
T.assert_type!(catch {1}, BasicObject)
