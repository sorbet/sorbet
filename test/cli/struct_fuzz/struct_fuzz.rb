# typed: true
::B=Struct.new:x
# these errors aren't actually what we're checking for, we just want to make sure sorbet doesn't crash on this input
# when no-stdlib is true.
