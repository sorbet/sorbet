# frozen_string_literal: true
# typed: strong
# compiled: true
5000.times do
  100.times do
    {"xxxx"=>"yyyy"}
  end
  GC.start
end
