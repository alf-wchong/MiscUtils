# SFDC / Certinia Timecards Exporter (Pattern B: host browser)

This version **does not include noVNC or a browser in the container**.
Instead, you authenticate using a flow that lets you open a URL in your **host browser**.

What you get:
- Fedora + Salesforce CLI (`sf`)
- Persistent auth volume (`/home/app/.sf`)
- Helper commands to query/export Certinia timecards

## Start

```bash
docker compose up --build
```

This drops you into a shell inside the container.

## Login (recommended)

### 1) Device login (host browser)
In the container shell:

```bash
sf-login-device
```

It will print a login URL + code. Open the URL in your **host browser**, enter the code, finish SSO/MFA.
Auth is saved to `/home/app/.sf` (persisted volume), so you usually do this once.

### 2) Web login (fallback)
If device login is blocked by org policy, try:

```bash
sf-login-web
```

Depending on your environment, it may:
- print an auth URL you can open on the host, **or**
- attempt to open a browser (which won’t work in this pattern).

If it tries to open a browser, we can switch to a URL-output flow or an auth-file import approach.

## Query + export

```bash
sf-set-default
get-timecards
export-timecards-md > /home/app/work/timecards.md
```

## Env vars

- `SF_INSTANCE_URL` (default `https://login.salesforce.com`)
- `SF_TARGET_ORG` (default `devhub`)
- `SF_LOGIN_TIMEOUT` (default `600`)
