# Expand the Root Logical Volume Before Installing DOXiS

| Item | Value |
|------|-------|
| Product | DOXiS ECM |
| Platform | Red Hat Enterprise Linux 8 / 9 / 10 |
| Severity | Medium |
| Category | Deployment Best Practice |
| Affected Components | Linux LVM, XFS, DOXiS Installation |
| Audience | Linux Administrators, Solution Architects, Deployment Engineers |

---

# Summary

Some Infrastructure as Code (IaC) images provision the operating system using a relatively small root logical volume (for example, **2 GB**) while leaving a significant amount of storage unallocated within the operating system Volume Group (`rootvg`).

Although technically valid, this configuration can cause DOXiS installations to fail due to insufficient free space despite tens of gigabytes remaining available inside the volume group.

This bulletin recommends expanding the root logical volume **before beginning any DOXiS installation**.

---

# Symptoms

Administrators may observe one or more of the following symptoms:

- DOXiS installation fails unexpectedly.
- Package extraction fails.
- Java archive extraction fails.
- Installation media cannot be unpacked.
- Operating system reports:

```text
No space left on device
```

or

```text
Filesystem      Size Used Avail Use%
/               2G   2G    0   100%
```

---

# Root Cause

The operating system has been provisioned with a small root logical volume while leaving free extents available within the Volume Group.

Example:

```bash
sudo vgs
```

```text
VG      VSize    VFree
rootvg  62.50g   37.50g
```

Although the server physically contains approximately 64 GB of operating system storage, only 2 GB has been allocated to the root filesystem.

---

# Resolution

Expand the root logical volume before installing DOXiS.

This operation is performed online and does **not** require a reboot.

---

# Validation

Verify the current root filesystem.

```bash
df -h /
```

Example:

```text
Filesystem                   Size Used Avail Use%
/dev/mapper/rootvg-rootlv     2G  2G    0   100%
```

Verify the Volume Group.

```bash
sudo vgs
```

Example:

```text
VG      VSize    VFree
rootvg  62.50g   37.50g
```

If `VFree` is greater than zero, the root filesystem should be expanded.

---

# Procedure

Expand the root logical volume to consume all remaining free extents.

```bash
sudo lvextend -r -l +100%FREE /dev/rootvg/rootlv
```

Explanation:

| Option | Meaning |
|---------|---------|
| `-r` | Automatically resize the XFS filesystem after extending the logical volume |
| `-l +100%FREE` | Allocate all remaining free extents within the Volume Group |

No reboot is required.

---

# Verification

Verify the logical volume.

```bash
sudo lvs
```

Expected:

```text
LV       VG      LSize
rootlv   rootvg 39.50g
```

Verify the filesystem.

```bash
df -h /
```

Expected:

```text
Filesystem                   Size Used Avail Use%
/dev/mapper/rootvg-rootlv     40G 2G    38G   6%
```

---

# If the Root Filesystem Is Already Full

If the root filesystem reaches 100% utilization before it is expanded, `lvextend` may fail because LVM cannot create temporary metadata archive files.

Typical error:

```text
Couldn't create temporary archive name.
```

This does **not** indicate a problem with LVM.

It indicates there is insufficient free space on the root filesystem to write LVM metadata during the resize operation.

## Resolution

Temporarily free space on the root filesystem.

Typical approaches include:

- moving installation media to another filesystem
- moving temporary deployment artifacts
- deleting temporary files

Once several hundred megabytes have been recovered, rerun:

```bash
sudo lvextend -r -l +100%FREE /dev/rootvg/rootlv
```

---

# Deployment Sequence

The recommended deployment sequence is:

```text
Provision VM
        │
        ▼
Validate storage
        │
        ▼
Expand root logical volume
        │
        ▼
Verify filesystem capacity
        │
        ▼
Install DOXiS
        │
        ▼
Configure repository storage
```

This sequence prevents installation failures caused by an undersized root filesystem.

---

# Best Practice

Where possible, infrastructure templates should allocate the entire operating system disk to the root logical volume during provisioning.

If infrastructure policy intentionally leaves unallocated extents within `rootvg`, deployment procedures should always expand the root logical volume before installing DOXiS.

---

# Related Documentation
- [Red Hat Enterprise Linux 9 – Configuring and managing logical volumes](https://docs.redhat.com/en/documentation/red_hat_enterprise_linux/9/html-single/configuring_and_managing_logical_volumes/index?utm_source=chatgpt.com)
