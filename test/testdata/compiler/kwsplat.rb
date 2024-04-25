# compiled: true
# typed: true
# frozen_string_literal: true

args = {x: 'hello', y: 'world'}
puts a: 1, **args, x: 'goodbye'

puts a: 0, **args, b: 1, c: 2, d: 3, e: 4, f: 5, g: 6, h: 7, i: 8, j: 9, k: 10, **args, x: 'goodbye'
