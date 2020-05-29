# typed: false

# We used to implement non_forcing_is_a? as an intrinsic method in calls.cc,
# which meant that we had the same `typed: false` behavior as `T.reveal_type`
# did (complaining that you can't use it at that strictness level).
#
# But now it lives elsewhere, so being in a `typed: false` file is fine.

T::NonForcingConstants.non_forcing_is_a?(self, '::Integer')
T::NonForcingConstants.non_forcing_is_a?(self, '::DoesntExist') # error: Unable to resolve constant `::DoesntExist`
