# typed: true
x = <<~EOF
 A
	Z
EOF

# Compare desugar output by analogy with VM output:
p x # => "A\n\tZ\n"
