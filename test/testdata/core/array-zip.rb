# frozen_string_literal: true
# typed: true

x = [1, 3, 5].cycle
# We used to assume #zip only took Arrays and therefore Sorbet used to crash here.
a = [2, 4, 6].zip(x)
