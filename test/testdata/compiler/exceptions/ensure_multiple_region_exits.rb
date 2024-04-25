# typed: true
# compiled: true
# frozen_string_literal: true

def testcase
  begin
    raise 'start'
  ensure
    begin
      raise 'continue'
    rescue
      raise 'real exception'
    end
  end
end
