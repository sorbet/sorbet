# typed: true
class Date < Object
  include Comparable

  ABBR_DAYNAMES = T.let(T.unsafe(nil), Array)
  ABBR_MONTHNAMES = T.let(T.unsafe(nil), Array)
  DAYNAMES = T.let(T.unsafe(nil), Array)
  ENGLAND = T.let(T.unsafe(nil), Integer)
  GREGORIAN = T.let(T.unsafe(nil), Float)
  ITALY = T.let(T.unsafe(nil), Integer)
  JULIAN = T.let(T.unsafe(nil), Float)
  MONTHNAMES = T.let(T.unsafe(nil), Array)
end

class Date::Infinity < Numeric
end
