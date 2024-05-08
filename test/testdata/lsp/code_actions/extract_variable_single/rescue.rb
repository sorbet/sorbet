# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

begin
  1 + 11
#     ^^ apply-code-action: [A] Extract Variable
rescue Errno::ENOENT
  2 + 22
#     ^^ apply-code-action: [B] Extract Variable
rescue ArgumentError
  3 + 33
#     ^^ apply-code-action: [C] Extract Variable
else
  4 + 44
#     ^^ apply-code-action: [D] Extract Variable
ensure
  5 + 55
#     ^^ apply-code-action: [E] Extract Variable
end

begin
ensure
  6 + 66
#     ^^ apply-code-action: [F] Extract Variable
end
