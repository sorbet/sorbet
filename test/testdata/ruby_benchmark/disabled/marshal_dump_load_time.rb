# typed: true
100000.times { Marshal.load(Marshal.dump(Time.now)) }
