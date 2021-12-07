#!/usr/bin/env bash

set -euo pipefail

test_dir="$(dirname "${BASH_SOURCE[0]}")"
root_dir="${test_dir}/../.."

# shellcheck source=SCRIPTDIR/../../logging.sh
source "${root_dir}/test/logging.sh"

usage() {
  info "Usage: ./new_test.sh <test-name>"
  exit 1
}

new_test_name="${1/\//-}"
if [[ "${new_test_name}" == "" ]]; then
  usage
fi

new_test_path="${test_dir}/${new_test_name}"
if [[ -d "${new_test_path}" ]]; then
  error "Error: CLI test \"${new_test_name}\" already exists"
  usage
fi

info "Creating ${new_test_path}"
mkdir "${new_test_path}"

info "Adding expected.out"
touch "${new_test_path}/expected.out"

info "Adding test.sh"
cat > "${new_test_path}/test.sh" << EOF
#!/usr/bin/env bash

set -euo pipefail

# NOTE: Sorbet and Sorbet Ruby are already in the PATH
EOF
chmod +x "${new_test_path}/test.sh"

success "Done"
