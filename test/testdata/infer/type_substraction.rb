# typed: true
sig(
  source: T.any(Integer, String, NilClass)
)
.returns(T.nilable(String))
def m1(source:)
  if source.is_a?(Integer)         
    nil
  else
    source
  end
end


sig(
  source: T.any(File, String, NilClass)
)
.returns(T.nilable(String))
def m2(source:)
  if source.is_a?(Enumerable)         
    nil
  else
    source
  end
end

sig(
 source: T.any([Integer], String, NilClass)
)
.returns(T.nilable(String))
def m3(source:)
 if source.is_a?(Enumerable)         
   nil
 else
   source
 end
end

module Mod; end;

sig(
    source: T.any(T.all(Integer, Mod), String, NilClass)
)
.returns(T.nilable(String))
def m4(source:)
 if source.is_a?(Integer)
   nil
 else
   source
 end
end
