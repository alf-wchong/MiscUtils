# **Operational Rules** 
To ensure you *don’t lose work*, plus the best way to handle rebuilds and backups.

## What is safe vs what deletes your “VM”

### Safe (state preserved)

* `podman stop rhel9vm` / `podman start rhel9vm`
* `podman restart rhel9vm`
* Rebooting WSL2 (container will stop; when you start it again, volumes still hold state)

### Destructive to container filesystem (but volumes still safe)

* `podman rm rhel9vm`
  You lose the container’s writable layer (anything NOT in volumes), but your persisted data under:

  * `/etc` (rhel9vm_etc)
  * `/var` (rhel9vm_var)
  * `/home` (rhel9vm_home)
  * `/opt/SER` (rhel9vm_opt_ser)
    will remain.

### Destructive to everything (don’t do this unless you intend to wipe)

* `podman volume rm rhel9vm_*`
* `podman system prune --volumes` (or any prune that includes volumes)
* manually deleting Podman storage

## The key idea

Your system is now “VM-like” because the important bits are on volumes. The container itself is basically a bootloader.

* **Put Doxis install under `/opt/SER`** ✅ (already persisted)
* Logs and runtime data under **`/var`** ✅
* Config under **`/etc`** ✅
* User state under **`/home`** ✅

This means you can **recreate the container image** (patches, baseline changes) without reinstalling Doxis, as long as `/opt/SER` is the volume of record.

## Recommended workflow for “upgrading the base OS image”

When you change the Containerfile and rebuild `localhost/rhel9vm:ubi9`:

1. Stop and remove the container (not volumes):

```bash
podman stop rhel9vm
podman rm rhel9vm
```

2. Re-run the same `podman run ...` command you used before (mounting the same volumes).

The system comes back with the new base image, and your Doxis install/config/state remains because it’s on volumes.

## Backups and “savepoints” (highly recommended for COTS practice)

### Backup volumes (captures the real VM state)

```bash
podman volume export rhel9vm_etc > rhel9vm_etc.tar
podman volume export rhel9vm_var > rhel9vm_var.tar
podman volume export rhel9vm_home > rhel9vm_home.tar
podman volume export rhel9vm_opt_ser > rhel9vm_opt_ser.tar
```

### Restore (to a new empty volume)

```bash
podman volume create rhel9vm_opt_ser
podman volume import rhel9vm_opt_ser < rhel9vm_opt_ser.tar
```

(You can do the same for the other volumes.)

## One important nuance about persisting `/etc`

Persisting `/etc` makes it very VM-ish (and is fine for your lab), but it also means:

* if you rebuild the image with newer defaults, your old `/etc` may override them.

That’s not wrong — just be aware when debugging “why is sshd config not changing” etc.

---

If you’re ready for the next step: do you want this target to be **single-node only**, or should we clone it into **2–3 nodes** (e.g., `rhel9vm1`, `rhel9vm2`, `rhel9vm3`) to practice Doxis topologies?
