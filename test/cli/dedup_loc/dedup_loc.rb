# typed: true

begin
rescue Exception => e
    T.assert_type!(e, NilClass)
end
