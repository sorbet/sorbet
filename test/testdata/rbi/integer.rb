# typed: strict

# Integer#[] accepts Integer index
T.assert_type!(5[1], Integer)

# Integer#[] accepts Integer index and length arguments
T.assert_type!(5[1, 3], Integer)

# Integer#[] accepts Float index and length arguments
T.assert_type!(5[1.0, 3.0], Integer)

# Integer#[] accepts Integer Range as argument
T.assert_type!(5[1..2], Integer)

# Integer#[] accepts Float Range as argument
T.assert_type!(5[1.0..2.0], Integer)

# Integer#[] accepts beginless Range as argument
T.assert_type!(5[nil..2], Integer)

# Integer#[] accepts endless Range as argument
T.assert_type!(5[1..nil], Integer)
