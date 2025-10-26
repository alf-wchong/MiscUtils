# Database & Search Analytics Stack

This repository provides a ready-to-run Docker Compose stack featuring:

- **PostgreSQL** (latest) – Relational database
- **pgAdmin 4** (latest) – Database management and GUI for PostgreSQL
- **OpenSearch** (latest, custom image with `analysis-icu` and `analysis-smartcn` plugins preinstalled)
- **Grafana** (latest) – Analytics & visualization, connected to OpenSearch
- **Cerebro** (latest) – Cluster admin UI for OpenSearch

All services use **named Docker volumes for persistent data** and are networked together via a custom Docker network.

---

## Stack Features

- **Fully containerized analytics and search backend**
- **Custom OpenSearch image** (`opensearch-analysisbakedin:latest`) pre-baked with popular analysis plugins
- **pgAdmin** auto-configured to connect to PostgreSQL on start
- **Grafana** auto-configured to connect to OpenSearch on start
- **Cerebro** auto-configured to connect to OpenSearch
- **All credentials and configuration** via environment file (`.env`)
- **Data persistence** using Docker named volumes

---

## Files Provided

| File                 | Purpose                                                   |
|----------------------|-----------------------------------------------------------|
| `Dockerfile`         | Builds custom OpenSearch image with analysis plugins      |
| `docker-compose.yml` | Orchestrates the services and named volumes               |
| `.env`               | Stores all secrets and config as environment variables    |
| `start_stack.sh`     | Bash script to build image, create volumes, and start stack (optional) |
| `README.md`          | This guide                                                |

---

## Requirements

- [Docker Desktop](https://www.docker.com/products/docker-desktop/) (tested with Docker CLI v28.5.1)
- Docker Compose plugin (`docker compose` command)
- Unix shell (for bash script)

---

## Usage

### 1. Clone and Setup

```bash
git clone <your-repo-url>
cd <your-repo-directory>
```
### 2. Fill In Secrets

Edit the `.env` file and set all credential placeholders to secure, real values. Example:

```bash
POSTGRES_USER=your_db_user POSTGRES_PASSWORD=your_db_password POSTGRES_DB=your_db_name

PGADMIN_DEFAULT_EMAIL=admin@example.com PGADMIN_DEFAULT_PASSWORD=your_pgadmin_password

OPENSEARCH_USERNAME=admin OPENSEARCH_PASSWORD=your_opensearch_password

CEREBRO_SECRET=your_cerebro_secret_key

GF_SECURITY_ADMIN_USER=admin GF_SECURITY_ADMIN_PASSWORD=your_grafana_password
```

---

### 3. Build and Start the Stack

Make the script executable:

```bash
chmod +x start_stack.sh
```


Run the script (builds image, creates volumes, and starts all services):
```bash
docker build -t opensearch-analysisbakedin:latest .
docker compose up -d
```

---

### 4. Service URLs

| Service    | URL                    | Credentials (.env variables)                   |
|------------|------------------------|-----------------------------------------------|
| PostgreSQL | localhost:5432         | POSTGRES_USER, POSTGRES_PASSWORD              |
| pgAdmin    | http://localhost:8888  | PGADMIN_DEFAULT_EMAIL, PGADMIN_DEFAULT_PASSWORD|
| OpenSearch | http://localhost:9200  | OPENSEARCH_USERNAME, OPENSEARCH_PASSWORD      |
| Grafana    | http://localhost:3000  | GF_SECURITY_ADMIN_USER, GF_SECURITY_ADMIN_PASSWORD |
| Cerebro    | http://localhost:9000  | CEREBRO_SECRET                                |

Check running services:
```bash
docker compose ps
```

---

## File Details

#### Dockerfile

Builds the custom OpenSearch image with analysis plugins.

#### docker-compose.yml

- Uses named volumes for data
- Defines the custom network `dB-Search-Network`
- Exposes required ports
- Sets dependencies

#### .env

Populate with secure credentials for all services.

#### start_stack.sh

Automates building, volume creation, and starting the stack.

---

## Persistence

All data and configurations are stored in named volumes:

- `postgres_data`
- `pgadmin_data`
- `opensearch_data`
- `grafana_data`

---

## Customization

Add more configuration, dashboards, or initial provisioning using bind mounts or provisioning directories if needed.

---

## Stop and Remove

To stop all containers:
```bash
docker compose down
```

---

## Support

- View logs: `docker compose logs <service>`
- Check your `.env` variables
- Ensure no port conflicts on your machine

---

## License

???


