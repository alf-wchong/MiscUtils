# Passwordless `sudo` Using the `wheel` Group

This guide describes the standard Red Hat Enterprise Linux (RHEL) method for enabling passwordless `sudo` through membership in the `wheel` group.

This is the recommended approach for standalone systems and environments where sudo privileges are managed centrally through `/etc/sudoers`.

> **Scope**
>
> This document covers only the standard `wheel` group configuration.
>
> If your environment provisions individual user entries under `/etc/sudoers.d` (for example, through Active Directory, cloud-init, Ansible, Puppet, or other configuration management), refer instead to:
>
> [passwordless-sudo-named-user.md](./passwordless-sudo-named-user.md)

---

# Sanity Check

Before making any changes, determine whether your environment already manages users individually.

Run:

```bash
grep -R "<username>" /etc/sudoers.d
```

Replace `<username>` with the target login name.

If the command returns a matching entry, for example:

```text
/etc/sudoers.d/domain-admins:
kal-el ALL=(ALL) ALL
```

then **stop**.

A user-specific rule exists and will typically override the `wheel` configuration. Use the guide:

[`passwordless-sudo-named-user.md`](./passwordless-sudo-named-user.md)

If no matching entry is found, continue with this guide.

---

# Add the User to the `wheel` Group

Verify current membership.

```bash
id <username>
```

or

```bash
groups <username>
```

Example:

```text
uid=1001(kal-el) gid=1001(kal-el) groups=1001(kal-el),10(wheel)
```

If the user is not already a member of `wheel`, add them:

```bash
sudo usermod -aG wheel <username>
```

The user must log out and log back in before the new group membership becomes effective.

Verify:

```bash
id <username>
```

---

# Configure `/etc/sudoers`

Open the sudoers file using `visudo`.

```bash
sudo visudo
```

Locate the `wheel` entries.

Typical RHEL configuration:

```sudoers
%wheel ALL=(ALL) ALL
```

Enable passwordless `sudo` by adding or uncommenting:

```sudoers
%wheel ALL=(ALL) NOPASSWD: ALL
```

If both entries exist, ensure the `NOPASSWD` entry appears **after** the password-required entry.

Example:

```sudoers
%wheel ALL=(ALL) ALL
%wheel ALL=(ALL) NOPASSWD: ALL
```

Save and exit.

---

# Validate the Configuration

Always verify the syntax.

```bash
sudo visudo -c
```

Expected:

```text
/etc/sudoers: parsed OK
```

---

# Verify Effective Permissions

Run:

```bash
sudo -l
```

Expected output should include:

```text
(ALL) NOPASSWD: ALL
```

---

# Test Passwordless `sudo`

Invalidate any cached credentials.

```bash
sudo -k
```

Execute:

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

## User Still Prompted for Password

Check whether a user-specific rule exists.

```bash
grep -R "<username>" /etc/sudoers.d
```

If a matching entry is found, it may override the `wheel` configuration.

Use:

[`passwordless-sudo-named-user.md`](./passwordless-sudo-named-user.md)

---

## Verify Group Membership

```bash
id <username>
```

Ensure `wheel` appears in the group list.

If the user was recently added to the group, they must log out and back in.

---

## Verify the `wheel` Rule

Confirm the `NOPASSWD` entry exists.

```bash
grep wheel /etc/sudoers
```

Expected:

```sudoers
%wheel ALL=(ALL) ALL
%wheel ALL=(ALL) NOPASSWD: ALL
```

---

## Validate the Sudoers Files

```bash
sudo visudo -c
```

Always validate after making changes.

---

# Notes

The `wheel` group is the standard RHEL mechanism for delegating administrative privileges.

When no user-specific entries exist under `/etc/sudoers.d`, membership in `wheel` combined with:

```sudoers
%wheel ALL=(ALL) NOPASSWD: ALL
```

provides passwordless `sudo` for every member of the group.

This approach is simple, easy to maintain, and is generally preferred unless your organization manages sudo permissions on a per-user basis.
