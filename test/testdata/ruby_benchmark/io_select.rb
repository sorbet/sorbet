# frozen_string_literal: true
# typed: strong
# compiled: true
# IO.select performance

w = [ IO.pipe[1] ];

nr = 1000000
nr.times {
  IO.select nil, w
}

