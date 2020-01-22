# typed: strict

require "date"

T.assert_type!(Date.today, Date)
T.assert_type!(Date.today + 1, Date)
