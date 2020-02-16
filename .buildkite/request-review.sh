#!/usr/bin/env bash

# We will probably have to make sure this logic lives directly in pipeline.sh
# (not in a separate script) because we only check the hash of pipeline.sh, and
# this script will need to be run for every pull request before it's been
# unblocked.
#
# This script will have a problem where if the branch build finishes before the
# pull request is opened, no reviewer will be assigned to the pull request.

generate_query() {
  local pull_request_number="$1"
  # Outputs a JSON string value (i.e., output is contained inside "...")
  # whose contents are stringified JSON (quotes, newlines, etc. are escaped)
  jq --raw-input --slurp --raw-output '@json' <<EOF
{
  organization(login: "sorbet") {
    repository(name: "sorbet") {
      pullRequest(number: $pull_request_number) {
        reviewRequests {
          totalCount
        }
        reviews {
          totalCount
        }
      }
    }
  }
}
EOF
}

if [ "$BUILDKITE_PULL_REQUEST" != "false" ]; then
  api_result="$(mktemp)"
  # shellcheck disable=SC2064
  trap "rm -f '$api_result'" EXIT

  # Uses .netrc credentials
  curl "https://api.github.com/graphql" -X POST -d "{\"query\": $(generate_query "$BUILDKITE_PULL_REQUEST")}" > "$api_result"

  if [ "$(jq '.data.organization.repository.pullRequest.reviews.totalCount' "$api_result")" -eq 0 ]; then
    echo "Pull request already reviewed by someone once; skipping review request"
    exit 0
  fi

  if [ "$(jq '.data.organization.repository.pullRequest.reviewRequests.totalCount' "$api_result")" -eq 0 ]; then
    echo "Pull request already has a reviewer requested; not requesting another one"
    exit 0
  fi

  if [ "$BUILDKITE_PULL_REQUEST_REPO" = "git://github.com/sorbet/sorbet.git" ]; then
    # My thinking right now is that we'd delete the CODEOWNERS file,
    # and have internal contributors request review from someone directly.
    #
    # Alternatively, we can use this check to determine which team to request review from.
    echo "Pull request opened directly against sorbet/sorbet; skipping review request"
    exit 0
  fi

  echo "Requesting review from @sorbet/runner"

  # Uses .netrc credentials
  curl "https://api.github.com/repos/sorbet/sorbet/pulls/$BUILDKITE_PULL_REQUEST/requested_reviewers" \
    -X POST \
    -H "Content-Type: application/json" \
    -d '{ "team_reviewers": [ "runner" ] }'
fi

