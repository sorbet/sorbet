# typed: true

T.reveal_type(Integer.===(T.unsafe(nil))) # error: Revealed type: `T::Boolean`
