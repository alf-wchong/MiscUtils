# Prompt:

Please create a working Docker-based application stack for database and search analytics, using the following specifications and requirements:
---

## Stack Overview:

* PostgreSQL (latest stable)
* pgAdmin (latest stable; admin user, connected to PostgreSQL on startup, exposed as host:8888)
* OpenSearch (latest stable; **custom image named opensearch-analysisbakedin:latest with plugins analysis-icu and analysis-smartcn baked in**)
* Grafana (latest stable; admin user, configured to connect to OpenSearch on startup)
* Cerebro (latest stable; admin user, connected to OpenSearch on startup)
---

## Requirements:

1. Compose three files:
   - A `Dockerfile` to create the custom OpenSearch image, pre-installing the analysis-icu and analysis-smartcn plugins, based on the latest stable OpenSearch official image. The image should be tagged as `opensearch-analysisbakedin:latest`.
   - A `docker-compose.yml` that references this custom OpenSearch image and orchestrates all listed services above, using:
     - Named volumes for persistence (not bind mounts).
     - A single Docker network named `db-Search-Network`.
     - All services exposed to the host with standard 1:1 port mapping, except pgAdmin, which should map container port 80 to host port 8888.
     - Each service must use `restart: unless-stopped` policy.
     - Environment variables provided via a `.env` file for all passwords, admin credentials, and config values (use clear placeholders).
     - pgAdmin should be preconfigured to connect to the local PostgreSQL database at startup.
     - Cerebro and Grafana should be preconfigured to connect to OpenSearch at startup.
     - Use official images (latest) wherever possible for all services except the custom OpenSearch image.
     - Compose file format must be compatible with Docker Compose v2 (at least version 3.8). Tested compatible with Docker CLI version 28.5.1.
   - An `.env` file with clear placeholder variables for all necessary configuration items.

2. Do not use supplementary config files unless essential for connecting the monitoring/admin tools to their backend services at startup (for example, using official provisioning).

## Request:
Output the full content for the three files (`Dockerfile`, `docker-compose.yml`, .`env`). The `docker-compose.yml` should reference the custom image for OpenSearch and configure all settings as detailed above.

Please use clear comments in the files where appropriate, but do not add extra explanation outside of the code blocks.

Before starting, verify if any additional clarifications are needed for correct inter-service connectivity or for provisioning Grafana data sources, pgAdmin server registration, or Cerebro auto-connect functionality. Do not make assumptions; prompt for any missing details first.

---

## Deliverables:

- `Dockerfile` for custom OpenSearch image named `opensearch-analysisbakedin:latest`
- `docker-compose.yml`
- .`env` (with placeholders only)

---

Do not output anything except the three requested file contents unless additional clarification is required.
