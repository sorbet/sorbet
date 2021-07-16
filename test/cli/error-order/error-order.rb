# typed: true

if T.unsafe(nil)
  T.reveal_type('this error first')
else
  T.reveal_type('this error second')
end
