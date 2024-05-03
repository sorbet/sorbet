# frozen_string_literal: true
# typed: true
# compiled: true

def no_block
  h1 = {q: 99, z: 48, t: 97, p: 100}
  e = h1.select
  puts (e.each do |k, v|
          puts "#{k} => #{v}"
          v%2==0 || k==:q
        end)
end

no_block


def with_block
  h1 = {q: 99, z: 48, t: 97, p: 100}
  puts (h1.select do |k, v|
          puts "#{k} => #{v}"
          v%2==0 || k==:q
        end)
end

with_block

