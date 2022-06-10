# typed: true

maybe_match = /foo/.match('foo')
T.reveal_type(maybe_match) # error: type: `T.nilable(MatchData)`

/foo/.match('foo') do |m|
  T.reveal_type(m) # error: type: `MatchData`
end

T.reveal_type(/foo/.match?('foo'))  # error: type: `T::Boolean`
T.reveal_type(/foo/.match?('foo', 1))  # error: type: `T::Boolean`

T.reveal_type(Regexp.compile('foo')) # error: type: `Regexp`
T.reveal_type(Regexp.compile('foo', Regexp::EXTENDED | Regexp::IGNORECASE)) # error: type: `Regexp`
T.reveal_type(Regexp.compile(/foo/)) # error: type: `Regexp`
