# typed: true

class TestData
    DuplicateData1 = Data.define(:foo, :foo) # error: Duplicate member 'foo' in Data definition
  
    DuplicateData2 = Data.define(:foo, 'foo') # error: Duplicate member 'foo' in Data definition
  
    DuplicateData3 = Data.define("FooDataDup", :foo, :bar, :foo) # error: Duplicate member 'foo' in Data definition
  
    ValidData1 = Data.define(:foo, :bar)
  
    ValidData2 = Data.define("FooData", :foo, :bar)
  
    ValidData3 = Data.define(:foo, "bar")
  end
  