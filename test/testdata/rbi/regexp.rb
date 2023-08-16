# typed: true

maybe_match = /foo/.match('foo')
T.reveal_type(maybe_match) # error: type: `T.nilable(MatchData)`

if maybe_match
  b1 = maybe_match.begin(0)
  T.reveal_type(b1) # error: type: `Integer`
  e1 = maybe_match.end(0)
  T.reveal_type(e1) # error: type: `Integer`

  # These are nonsensical because there are no capture groups in
  # the original regexp, but Sorbet doesn't know that.
  b2 = maybe_match.begin(:nope)
  T.reveal_type(b2) # error: type: `Integer`
  e2 = maybe_match.end(:nope)
  T.reveal_type(e2) # error: type: `Integer`

  b3 = maybe_match.begin("nope")
  T.reveal_type(b3) # error: type: `Integer`
  e3 = maybe_match.end("nope")
  T.reveal_type(e3) # error: type: `Integer`
end

/foo/.match('foo') do |m|
  T.reveal_type(m) # error: type: `MatchData`
end

T.reveal_type(/foo/.match?('foo'))  # error: type: `T::Boolean`
T.reveal_type(/foo/.match?('foo', 1))  # error: type: `T::Boolean`

T.reveal_type(Regexp.compile('foo')) # error: type: `Regexp`
T.reveal_type(Regexp.compile('foo', Regexp::EXTENDED | Regexp::IGNORECASE)) # error: type: `Regexp`
T.reveal_type(Regexp.compile(/foo/)) # error: type: `Regexp`

T.reveal_type(Regexp.timeout) # error: type: `T.nilable(Float)`
T.reveal_type(Regexp.timeout = 3.0) # error: type: `Float(3.000000)`
T.reveal_type(Regexp.timeout) # error: type: `T.nilable(Float)`
T.reveal_type(Regexp.timeout = nil) # error: type: `NilClass`

Regexp::TimeoutError.new
