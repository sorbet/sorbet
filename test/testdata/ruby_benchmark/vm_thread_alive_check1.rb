# frozen_string_literal: true
# typed: strict
# compiled: true
5_000.times{
  t = Thread.new{}
  while t.alive?
    Thread.pass
  end
}
