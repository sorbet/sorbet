# frozen_string_literal: true
# typed: true

p 'starting test'

def interpreted_raise(&blk)
  p 'interpreted_raise'
  yield :return
end

def run
  compiled_ensure do
    compiled_rescue do
      compiled_ensure do
        p "running interpreted block"
        interpreted_raise do |val|
          return val
        end
      end
    end
  end
end
v = :unmodified
v = run

p v
p 'done'
