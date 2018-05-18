# typed: strict

begin
rescue Exception => e
    T.assert_type!(e, NilClass)
end
