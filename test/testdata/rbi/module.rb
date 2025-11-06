# typed: true

String.module_eval { puts self }
Enumerable.class_eval { |mod| puts mod }

T.assert_type!(Module.const_source_location('IO'), T.nilable([String, Integer]))
T.assert_type!(Module.const_source_location('Zlib'), T.nilable([String, Integer]))
T.assert_type!(Module.const_source_location('Zlib', true), T.nilable([String, Integer]))
T.assert_type!(Module.const_source_location(:Zlib, false), T.nilable([String, Integer]))
T.assert_type!(Module.private_class_method([:foo, :bar]), T::Module[T.anything])
T.assert_type!(Module.private_class_method(["foo", "bar"]), T::Module[T.anything])
T.assert_type!(Module.private_class_method(:foo), T::Module[T.anything])
T.assert_type!(Module.private_class_method("bar"), T::Module[T.anything])
