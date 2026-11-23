#!/bin/sh
# Offline smoke tests for the NetBSD backend.
#
# These checks do not need a running VM or a memory dump: they validate the
# shipped image configurations, the kernel symbol tables and the public API
# surface. The three runtime image tests live in test_netbsd.c and are
# registered with CTest as test_netbsd_image_<version>; they are skipped
# unless the corresponding LIBVMI_NETBSD_IMAGE_* variable points at a dump.

set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)

VERSIONS="9.3 10.1 10.99"

fail() {
    echo "FAIL: $1" >&2
    exit 1
}

for version in $VERSIONS; do
    dir="$SCRIPT_DIR/netbsd-$version"
    conf="$dir/libvmi.conf"
    sysmap="$dir/System.map"

    test -f "$conf" || fail "missing config for NetBSD $version"
    test -f "$sysmap" || fail "missing System.map for NetBSD $version"

    grep -q 'ostype.*=.*"NetBSD"' "$conf" \
        || fail "NetBSD $version config does not declare ostype NetBSD"

    for key in netbsd_pid netbsd_name netbsd_vmspace netbsd_pmap netbsd_pgd; do
        grep -q "$key" "$conf" \
            || fail "NetBSD $version config is missing $key"
    done

    # every configured symbol table must expose the process-list anchor
    grep -Eq '^[0-9a-fA-F]+[[:space:]]+[A-Za-z][[:space:]]+allproc$' "$sysmap" \
        || fail "allproc missing from NetBSD $version System.map"
done

# the public API and the build system must know about NetBSD
grep -q 'VMI_OS_NETBSD' "$REPO_ROOT/libvmi/libvmi.h" \
    || fail "VMI_OS_NETBSD not exposed in the public API"
grep -q 'vmi_get_netbsd_sysmap' "$REPO_ROOT/libvmi/libvmi.h" \
    || fail "vmi_get_netbsd_sysmap not exposed in the public API"
grep -q 'ENABLE_NETBSD' "$REPO_ROOT/CMakeLists.txt" \
    || fail "ENABLE_NETBSD not wired into the CMake build"

echo "NetBSD offline smoke tests passed for: $VERSIONS"
