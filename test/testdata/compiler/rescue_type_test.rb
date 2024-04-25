# frozen_string_literal: true
# typed: true
# compiled: true

begin
  x = T.let(T.unsafe(336), String)
  p x
rescue TypeError => exn
  p exn.message.split("\n").reject {|line| /\ACaller:/ =~ line}
end
