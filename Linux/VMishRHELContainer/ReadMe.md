# RHEL 9 “Pseudo-VM” with Podman + systemd for Ansible COTS Deployment Practice (WSL2 Fedora Controller)
A step-by-step guide to building a persistent RHEL 9 “pseudo-VM” using Podman on Fedora WSL2, running systemd and SSH inside a UBI9 container, and configuring Ansible for realistic single-node enterprise COTS (SER Doxis) deployment practice with durable volumes (including /opt/SER).

---

## 1) Install Podman (Fedora WSL2)

```bash
sudo dnf install -y podman podman-compose
podman --version
```

Expected: `podman version 5.x` (you had 5.7.1).

---

## 2) Validate Podman environment (sanity check)

```bash
podman info --debug | sed -n '1,120p'
```

You’re looking for:

* `rootless: true`
* `cgroupVersion: v2`
* `cgroupManager: systemd`
* runtime like `crun` with `+SYSTEMD`

---

## 3) Smoke test: systemd inside a UBI9 container

Run:

```bash
podman run --rm -it --systemd=always \
  --tmpfs /run --tmpfs /tmp \
  registry.access.redhat.com/ubi9/ubi:latest \
  bash -lc 'dnf -y install systemd && /usr/sbin/init'
```

This will “boot” and **not return** (systemd is running as PID 1).

In another terminal, confirm systemd is alive:

```bash
podman ps --latest
CID=<container_id>

podman exec -it $CID systemctl is-system-running
podman exec -it $CID systemctl list-units --type=service --state=running | head -n 30
```

Stop the smoke test container:

```bash
podman stop $CID
```

(It auto-removes because `--rm` was used.)

---

## 4) Create a reusable “RHEL9 VM target” image (UBI9 + systemd + ssh + python)

Create the working directory under your chosen location:

```bash
mkdir -p ~/DuPontDev/rhel9vm
cd ~/DuPontDev/rhel9vm
```

Create `Containerfile`:

```bash
cat > Containerfile <<'EOF'
FROM registry.access.redhat.com/ubi9/ubi:latest

ENV container=podman

# Install systemd + ssh + sudo + python (for ansible)
RUN dnf -y update && \
    dnf -y install systemd openssh-server sudo python3 && \
    dnf clean all

# Prepare sshd
RUN ssh-keygen -A && \
    mkdir -p /run/sshd

# Create ansible user
RUN useradd -m -s /bin/bash ansible && \
    echo "ansible ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/ansible && \
    chmod 0440 /etc/sudoers.d/ansible

# SSH hardening defaults for lab use
RUN sed -i 's/^#PermitRootLogin.*/PermitRootLogin no/' /etc/ssh/sshd_config && \
    sed -i 's/^#PasswordAuthentication.*/PasswordAuthentication no/' /etc/ssh/sshd_config && \
    sed -i 's/^#PubkeyAuthentication.*/PubkeyAuthentication yes/' /etc/ssh/sshd_config

# Some units don't make sense in a container; mask them
RUN systemctl mask \
      dev-hugepages.mount \
      sys-fs-fuse-connections.mount \
      systemd-logind.service \
      getty.target || true

EXPOSE 22

STOPSIGNAL SIGRTMIN+3

CMD ["/usr/sbin/init"]
EOF
```

Build the image:

```bash
podman build -t rhel9vm:ubi9 .
```

---

## 5) Create persistent volumes (VM-like durability)

We persisted:

* `/etc`
* `/var`
* `/home`
* `/opt/SER` (your required Doxis installation root)

```bash
podman volume create rhel9vm_etc
podman volume create rhel9vm_var
podman volume create rhel9vm_home
podman volume create rhel9vm_opt_ser
```

Verify:

```bash
podman volume ls | grep rhel9vm
```

---

## 6) Run the persistent systemd “pseudo-VM” container

```bash
podman run -d --name rhel9vm \
  --hostname rhel9vm \
  --systemd=always \
  --tmpfs /run --tmpfs /tmp \
  -v rhel9vm_etc:/etc \
  -v rhel9vm_var:/var \
  -v rhel9vm_home:/home \
  -v rhel9vm_opt_ser:/opt/SER \
  -p 2222:22 \
  localhost/rhel9vm:ubi9
```

Verify it’s up:

```bash
podman ps --filter name=rhel9vm
podman exec -it rhel9vm systemctl is-system-running
```

Expected: `running`.

---

## 7) Enable sshd under systemd (Ansible realism)

```bash
podman exec -it rhel9vm systemctl enable --now sshd
podman exec -it rhel9vm systemctl status sshd --no-pager -l | sed -n '1,25p'
```

Expected: `Active: active (running)`.

---

## 8) Create a dedicated SSH keypair for this pseudoVM and install it

Create the key:

```bash
ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519_rhel9vm -C "ansible@rhel9vm" -N ""
```

Install it into the container:

```bash
podman exec -it rhel9vm bash -lc 'mkdir -p /home/ansible/.ssh && chmod 700 /home/ansible/.ssh && chown -R ansible:ansible /home/ansible/.ssh'
podman cp ~/.ssh/id_ed25519_rhel9vm.pub rhel9vm:/home/ansible/.ssh/authorized_keys
podman exec -it rhel9vm bash -lc 'chown ansible:ansible /home/ansible/.ssh/authorized_keys && chmod 600 /home/ansible/.ssh/authorized_keys'
```

Test SSH:

```bash
ssh -i ~/.ssh/id_ed25519_rhel9vm -p 2222 \
  -o StrictHostKeyChecking=accept-new \
  ansible@localhost 'whoami && /usr/bin/hostname && systemctl is-system-running'
```

If `hostname` is missing, install baseline utilities:

```bash
podman exec -it rhel9vm bash -lc 'dnf -y install hostname procps-ng iproute iputils which less vim-minimal && dnf clean all'
```

---

## 9) Configure Ansible controller files

In your Ansible directory (you used `~/DuPontDev/Ansible`), create:

### `inventory.ini`

```bash
cat > inventory.ini <<'EOF'
[rhel9vms]
rhel9vm ansible_host=127.0.0.1 ansible_port=2222 ansible_user=ansible
EOF
```

### `ansible.cfg`

```bash
cat > ansible.cfg <<'EOF'
[defaults]
inventory = ./inventory.ini
host_key_checking = False
interpreter_python = auto_silent

[ssh_connection]
ssh_args = -i ~/.ssh/id_ed25519_rhel9vm -o ControlMaster=auto -o ControlPersist=60s
EOF
```

Test:

```bash
ansible rhel9vm -m ping
ansible rhel9vm -m command -a "systemctl is-system-running"
```

---

## 10) Prove persistence across container restarts (marker file test)

### Create marker files in persisted locations

```bash
ansible rhel9vm -m shell -a 'sudo mkdir -p /opt/SER && echo "SER-Doxis marker $(date -u)" | sudo tee /opt/SER/marker.txt >/dev/null'
ansible rhel9vm -m shell -a 'echo "home marker $(date -u)" | tee /home/ansible/home_marker.txt >/dev/null'
ansible rhel9vm -m shell -a 'sudo bash -lc "echo var marker $(date -u) > /var/tmp/var_marker.txt"'
ansible rhel9vm -m shell -a 'sudo bash -lc "echo etc marker $(date -u) > /etc/marker.txt"'
```

### “Power cycle” the pseudoVM

```bash
podman stop rhel9vm
podman start rhel9vm
```

### Re-check marker files

```bash
ansible rhel9vm -m shell -a 'sudo ls -l /opt/SER/marker.txt && sudo cat /opt/SER/marker.txt'
ansible rhel9vm -m shell -a 'ls -l /home/ansible/home_marker.txt && cat /home/ansible/home_marker.txt'
ansible rhel9vm -m shell -a 'sudo ls -l /var/tmp/var_marker.txt && sudo cat /var/tmp/var_marker.txt'
ansible rhel9vm -m shell -a 'sudo ls -l /etc/marker.txt && sudo cat /etc/marker.txt'
```

Expected: all files still exist with the correct contents, proving persistence through restarts.

---

If you want, I can consolidate this further into a single “bootstrap script” (controller-side) that builds the image, creates volumes, runs the container, and sets up SSH/Ansible in one go.
