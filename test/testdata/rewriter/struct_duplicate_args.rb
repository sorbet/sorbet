# typed: true

class TestStructs
  DuplicateStruct1 = Struct.new(:foo, :foo) # error: Duplicate member 'foo' in Struct definition

  DuplicateStruct2 = Struct.new(:foo, 'foo') # error: Duplicate member 'foo' in Struct definition

  DuplicateStruct3 = Struct.new("FooStructDup", :foo, :bar, :foo) # error: Duplicate member 'foo' in Struct definition

  DuplicateStruct4 = Struct.new(:foo, :bar, :foo, keyword_init: true) # error: Duplicate member 'foo' in Struct definition

  ValidStruct1 = Struct.new(:foo, :bar)

  ValidStruct2 = Struct.new("FooStruct", :foo, :bar)

  ValidStruct3 = Struct.new(:foo, "bar")

  ValidStruct4 = Struct.new(:foo, :bar, keyword_init: true)

end
