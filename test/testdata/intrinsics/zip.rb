# typed: true
s = [1].zip([2], [3])
T.reveal_type(s) # error: Revealed type: `T::Array[[Integer, T.nilable(Integer), T.nilable(Integer)]]`
