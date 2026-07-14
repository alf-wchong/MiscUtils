# Passwordless `sudo` for Individually Managed Users

This guide describes how to configure passwordless `sudo` for an individual user in environments where sudo privileges are managed through user-specific files under `/etc/sudoers.d`.

This is common in enterprise environments where accounts are provisioned through Active Directory, cloud-init, Ansible, Puppet, or other configuration management systems.

> **Scope**
>
> This document intentionally does **not** cover the standard `wheel` group configuration. See [`passwordless-sudo-wheel.md`](./passwordless-sudo-wheel.md) for the conventional RHEL approach.

---

# Sanity Check

Before proceeding, determine whether your environment already provisions individual sudo rules.

Run:

```bash
grep -R "<username>\|ALL=(ALL)" /etc/sudoers.d
```

Replace `<username>` with the target login name.

Example:

```text
/etc/sudoers.d/domain-admins:
kal-el ALL=(ALL) ALL
```

If the user already has an entry in `/etc/sudoers.d`, continue with this guide.

If no user-specific entry exists, your system is likely using the standard `wheel` configuration instead. Refer to:

```
[`passwordless-sudo-wheel.md`](./passwordless-sudo-wheel.md)
```

---

# Locate the Controlling File

Determine which sudoers include file defines the user's privileges.

```bash
grep -R "<username>" /etc/sudoers.d
```

Example:

```text
/etc/sudoers.d/domain-admins:
kal-el ALL=(ALL) ALL
```

---

# Edit the Rule

Always use `visudo`.

```bash
sudo visudo -f /etc/sudoers.d/domain-admins
```

Change:

```sudoers
kal-el ALL=(ALL) ALL
```

to:

```sudoers
kal-el ALL=(ALL) NOPASSWD: ALL
```

Save and exit.

---

# Verify the Change

```bash
grep "^<username>" /etc/sudoers.d/domain-admins
```

Example:

```text
kal-el ALL=(ALL) NOPASSWD: ALL
```

---

# Validate the Configuration

```bash
sudo visudo -c
```

Expected:

```text
/etc/sudoers: parsed OK
/etc/sudoers.d/domain-admins: parsed OK
```

---

# Test

Invalidate any cached sudo credentials.

```bash
sudo -k
```

Then execute:

```bash
sudo whoami
```

Expected output:

```text
root
```

No password prompt should appear.

---

# Troubleshooting

## View All Matching Rules

```bash
sudo -l
```

Example:

```text
User kal-el may run the following commands:

    (ALL) ALL
    (ALL) NOPASSWD: ALL
    (ALL) ALL
```

`sudo -l` lists every matching rule but does **not** indicate which one controls authentication.

---

## Find All Matching Rules

```bash
grep -R "<username>\|NOPASSWD\|ALL=(ALL)" \
    /etc/sudoers \
    /etc/sudoers.d
```

Look for user-specific entries appearing under `/etc/sudoers.d`.

---

## Validate Syntax

```bash
sudo visudo -c
```

Always validate after making changes.

---

## Check File Attributes

If the file cannot be modified:

```bash
lsattr /etc/sudoers.d/<file>
```

If an immutable (`i`) attribute is present:

```bash
sudo chattr -i /etc/sudoers.d/<file>
```

---

# Notes

A user-specific rule under `/etc/sudoers.d` overrides more general group-based rules in `/etc/sudoers`.

For example:

```sudoers
# /etc/sudoers
%wheel ALL=(ALL) NOPASSWD: ALL

# /etc/sudoers.d/domain-admins
kal-el ALL=(ALL) ALL
```

Although `kal-el` belongs to `wheel`, the later user-specific rule requires password authentication.

Changing the user-specific rule to:

```sudoers
kal-el ALL=(ALL) NOPASSWD: ALL
```

restores passwordless `sudo` for that user without modifying the global `wheel` policy.
