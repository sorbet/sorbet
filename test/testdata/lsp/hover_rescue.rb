# typed: true
extend T::Sig

begin
  5
rescue TypeError => e
  #                 ^ hover: TypeError
  T.reveal_type(e) # error: `TypeError`
  raise
end
