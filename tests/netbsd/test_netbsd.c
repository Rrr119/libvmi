/* The LibVMI Library is an introspection library that simplifies access to
 * memory in a target virtual machine or in a file containing a dump of
 * a system's physical memory.  LibVMI is based on the XenAccess Library.
 *
 * This file is part of LibVMI.
 *
 * LibVMI is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * LibVMI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with LibVMI.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Image tests for the NetBSD introspection backend.
 *
 * Each test case targets one supported NetBSD image (a physical memory
 * dump produced, for example, with `qemu-img`/`dump-memory` or xl save).
 * The image path is read from the LIBVMI_NETBSD_IMAGE_<VERSION> environment
 * variable and the matching LibVMI configuration is passed as a string
 * (VMI_CONFIG_STRING). When no image is provided the test is skipped, so
 * the suite stays green in CI environments without the proprietary images.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>
#include <libvmi/libvmi.h>
#include <check.h>

#define NETBSD_CONF_FMT \
    "netbsd-test {" \
    " ostype = \"NetBSD\";" \
    " sysmap = \"%s\";" \
    " netbsd_pid = %#llx;" \
    " netbsd_name = %#llx;" \
    " netbsd_vmspace = %#llx;" \
    " netbsd_pmap = %#llx;" \
    " netbsd_pgd = %#llx;" \
    "}"

struct netbsd_image_test {
    const char *env_name;     /**< env var holding the memory dump path */
    const char *sysmap;       /**< kernel symbol table shipped in the repo */
    unsigned long long pid;   /**< proc->p_pid offset */
    unsigned long long name;  /**< proc->p_comm offset */
    unsigned long long vmspace; /**< proc->p_vmspace offset */
    unsigned long long pmap;  /**< vmspace->vm_pmap offset */
    unsigned long long pgd;   /**< pmap->pm_cr3 offset */
};

static const struct netbsd_image_test image_tests[] = {
    {
        .env_name = "LIBVMI_NETBSD_IMAGE_9_3",
        .sysmap = "tests/netbsd/netbsd-9.3/System.map",
        .pid = 0x70,
        .name = 0x26c,
        .vmspace = 0xb0,
        .pmap = 0x40,
        .pgd = 0x28,
    },
    {
        .env_name = "LIBVMI_NETBSD_IMAGE_10_1",
        .sysmap = "tests/netbsd/netbsd-10.1/System.map",
        .pid = 0x78,
        .name = 0x274,
        .vmspace = 0xb8,
        .pmap = 0x48,
        .pgd = 0x28,
    },
    {
        .env_name = "LIBVMI_NETBSD_IMAGE_10_99",
        .sysmap = "tests/netbsd/netbsd-10.99/System.map",
        .pid = 0x80,
        .name = 0x27c,
        .vmspace = 0xc0,
        .pmap = 0x50,
        .pgd = 0x30,
    },
};

static void
run_image_test(const struct netbsd_image_test *t)
{
    const char *only = getenv("LIBVMI_NETBSD_TEST_CASE");
    if (only && strcmp(only, t->env_name)) {
        return;
    }

    const char *image = getenv(t->env_name);
    if (!image) {
        fprintf(stderr, "skip: %s not set\n", t->env_name);
        return;
    }

    char *config = g_strdup_printf(NETBSD_CONF_FMT, t->sysmap,
                                   t->pid, t->name, t->vmspace,
                                   t->pmap, t->pgd);

    vmi_instance_t vmi = NULL;
    status_t rc = vmi_init_complete(&vmi, (void *)image, VMI_INIT_DOMAINNAME,
                                    NULL, VMI_CONFIG_STRING,
                                    (void *)config, NULL);
    ck_assert_msg(VMI_SUCCESS == rc, "%s", "vmi_init_complete failed on NetBSD image");
    ck_assert_msg(VMI_OS_NETBSD == vmi_get_ostype(vmi), "%s",
                  "image was not recognized as NetBSD");

    addr_t allproc = 0;
    rc = vmi_translate_ksym2v(vmi, "allproc", &allproc);
    ck_assert_msg(VMI_SUCCESS == rc, "%s", "allproc symbol lookup failed");
    ck_assert_msg(allproc != 0, "%s", "allproc resolved to address zero");

    addr_t offset = 0;
    ck_assert_msg(VMI_SUCCESS == vmi_get_offset(vmi, "netbsd_pid", &offset),
                  "%s", "netbsd_pid offset not exposed");
    ck_assert_msg(offset == (addr_t)t->pid, "%s", "netbsd_pid offset mismatch");
    ck_assert_msg(VMI_SUCCESS == vmi_get_offset(vmi, "netbsd_pgd", &offset),
                  "%s", "netbsd_pgd offset not exposed");
    ck_assert_msg(offset == (addr_t)t->pgd, "%s", "netbsd_pgd offset mismatch");

    const char *profile = vmi_get_os_profile_path(vmi);
    ck_assert_msg(profile != NULL, "%s", "vmi_get_os_profile_path returned NULL");
    ck_assert_msg(0 == strcmp(profile, t->sysmap), "%s",
                  "OS profile path does not match configured sysmap");

    addr_t pid0_dtb = 0;
    rc = vmi_pid_to_dtb(vmi, 0, &pid0_dtb);
    ck_assert_msg(VMI_SUCCESS == rc, "%s", "vmi_pid_to_dtb failed for proc0");

    vmi_pid_t resolved_pid = -1;
    rc = vmi_dtb_to_pid(vmi, pid0_dtb, &resolved_pid);
    ck_assert_msg(VMI_SUCCESS == rc, "%s", "vmi_dtb_to_pid failed");
    ck_assert_msg(resolved_pid == 0, "%s", "DTB did not resolve back to pid 0");

    vmi_destroy(vmi);
    g_free(config);
}

START_TEST(test_netbsd_9_3_image)
{
    run_image_test(&image_tests[0]);
}
END_TEST

START_TEST(test_netbsd_10_1_image)
{
    run_image_test(&image_tests[1]);
}
END_TEST

START_TEST(test_netbsd_10_99_image)
{
    run_image_test(&image_tests[2]);
}
END_TEST

static Suite *
netbsd_suite(void)
{
    Suite *s = suite_create("NetBSD images");
    TCase *tc = tcase_create("image introspection");
    tcase_add_test(tc, test_netbsd_9_3_image);
    tcase_add_test(tc, test_netbsd_10_1_image);
    tcase_add_test(tc, test_netbsd_10_99_image);
    suite_add_tcase(s, tc);
    return s;
}

int
main(void)
{
    int number_failed = 0;
    SRunner *sr = srunner_create(netbsd_suite());
    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
