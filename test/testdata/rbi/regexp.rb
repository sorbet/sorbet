# typed: true

maybe_match = /foo/.match('foo')
T.reveal_type(maybe_match) # error: type: `T.nilable(MatchData)`

/foo/.match('foo') do |m|
  T.reveal_type(m) # error: type: `MatchData`
end
