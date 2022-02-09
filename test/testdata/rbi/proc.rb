# typed: true

-> (x) { x }[123]

my_proc = Proc.new {}
T.reveal_type(my_proc) # error: type: `Proc`
