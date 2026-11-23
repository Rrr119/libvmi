# NetBSD image tests

These tests exercise the LibVMI NetBSD introspection backend against three
supported guest images:

| Image                    | Profile directory   | Image variable                     |
|--------------------------|---------------------|------------------------------------|
| NetBSD 9.3 (amd64)       | `netbsd-9.3/`       | `LIBVMI_NETBSD_IMAGE_9_3`          |
| NetBSD 10.1 (amd64)      | `netbsd-10.1/`      | `LIBVMI_NETBSD_IMAGE_10_1`         |
| NetBSD 10.99/-current    | `netbsd-10.99/`     | `LIBVMI_NETBSD_IMAGE_10_99`        |

Each directory contains:

- `libvmi.conf`: LibVMI configuration (`ostype = "NetBSD"` plus the
  `netbsd_pid`, `netbsd_name`, `netbsd_vmspace`, `netbsd_pmap` and
  `netbsd_pgd` offsets for that kernel).
- `System.map`: the kernel symbol table, including the `allproc` anchor used
  to walk the process list.

## Running the tests

The runtime tests consume a physical memory dump of the booted image
(for example a QEMU `dump-memory` file or a Xen `xl save` stream):

```sh
export LIBVMI_NETBSD_IMAGE_9_3=/path/to/netbsd-9.3.vmem
export LIBVMI_NETBSD_IMAGE_10_1=/path/to/netbsd-10.1.vmem
export LIBVMI_NETBSD_IMAGE_10_99=/path/to/netbsd-current.vmem
ctest -R netbsd --output-on-failure
```

When an image variable is unset the corresponding test case skips itself,
so a plain `ctest` run stays green without the (large, proprietary) dumps.

An offline smoke test, `test_netbsd_offline`, always runs and validates the
three configurations, the symbol tables and the public API wiring without
needing an image:

```sh
sh tests/netbsd/run_netbsd_tests.sh
```

## What is verified

For every image the test checks:

1. `vmi_init_complete()` succeeds and `vmi_get_ostype()` reports
   `VMI_OS_NETBSD`;
2. the `allproc` kernel symbol resolves through the configured `System.map`;
3. the `netbsd_*` offsets are exposed through the existing
   `vmi_get_offset()` API;
4. `vmi_get_os_profile_path()` returns the configured symbol table;
5. `vmi_pid_to_dtb()` / `vmi_dtb_to_pid()` round-trip the kernel process.
