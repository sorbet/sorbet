# typed: true

class ::Regexp::Parser
end

class ::Regexp::Parser::AbsoluteError < StandardError
end

class ::Regexp::AbsoluteScannerError < ::Regexp::Parser::AbsoluteError
end
