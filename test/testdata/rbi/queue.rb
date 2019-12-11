# typed: true

string_q = Queue[String].new
T.assert_type!(string_q.push('a'), Queue[String])
T.assert_type!(string_q.enq('b'), Queue[String])
T.assert_type!(string_q << 'c', Queue[String])
T.assert_type!(string_q.pop, String)
T.assert_type!(string_q.deq, String)
T.assert_type!(string_q.clear, Queue[String])
T.assert_type!(string_q.closed?, T::Boolean)
T.assert_type!(string_q.close, Queue[String])
