# frozen_string_literal: true
# typed: true

p 'starting test'

def interpreted_raise(&blk)
  p 'interpreted_raise'
  raise KeyError, 'throwing exception'
end

begin
  compiled_ensure do
    compiled_noop do
      compiled_rescue do
        compiled_ensure do
          p "running interpreted block"
          interpreted_raise do
            raise RuntimeError, "should not see this"
          end
        end
      end
    end
  end
rescue IndexError
  p 'outermost rescue'
end

p 'done'
