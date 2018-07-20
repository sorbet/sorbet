# typed: strict

Range.new(1, 10, true)
Range.new(1.0, 100.0, false)
Range.new('a', 'z')

(1..10)
(1.0...20.0)
('a'..'z')
(:a...:z)
