# typed: false

problematic_code rescue puts "rescued"
problematic_code rescue nil
problematic_code rescue raise rescue puts "rescued again"
