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

#ifndef OS_NETBSD_H_
#define OS_NETBSD_H_

#include "private.h"

struct netbsd_instance {
    char *sysmap; /**< system map file for domain's running kernel */

    addr_t pmap_offset; /**< vmspace->vm_pmap */

    addr_t vmspace_offset; /**< proc->p_vmspace */

    addr_t pid_offset; /**< proc->p_pid */

    addr_t pgd_offset; /**< pmap->pm_cr3 */

    addr_t name_offset; /**< proc->p_comm */
};
typedef struct netbsd_instance *netbsd_instance_t;

status_t netbsd_init(vmi_instance_t instance, GHashTable *config);

status_t netbsd_get_offset(vmi_instance_t vmi, const char* offset_name, addr_t *offset);

status_t netbsd_symbol_to_address(vmi_instance_t instance,
                                  const char *symbol, addr_t *__unused, addr_t *address);

char* netbsd_system_map_address_to_symbol(vmi_instance_t vmi, addr_t address,
        const access_context_t *ctx);

status_t netbsd_pid_to_pgd(vmi_instance_t vmi, vmi_pid_t pid, addr_t *dtb);

status_t netbsd_pgd_to_pid(vmi_instance_t vmi, addr_t pgd, vmi_pid_t *pid);

status_t netbsd_teardown(vmi_instance_t vmi);

#endif /* OS_NETBSD_H_ */
