# @typed
T.assert_type!(Struct.new(:a), Class)
# Class.new doesn't exist in our stdlib :(
T.assert_type!(Struct.new(:a).new(2), Object) # error: MULTI
