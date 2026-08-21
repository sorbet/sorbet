# typed: true

class Regexp::Parser
end

class Regexp::Parser::Error < StandardError
end

class Regexp::ScannerError < Regexp::Parser::Error
end
