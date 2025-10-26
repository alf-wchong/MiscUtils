#!/bin/bash

set -e

echo "Building custom OpenSearch image..."
docker build -t opensearch-analysisbakedin:latest .

echo "Creating named volumes if they don't exist..."
docker volume create postgres_data || true
docker volume create pgadmin_data || true
docker volume create opensearch_data || true
docker volume create grafana_data || true

echo "Starting services with Docker Compose..."
docker compose up -d

echo ""
echo "All services are starting..."
echo ""
echo "Quick connection info:"
echo "Postgres:          localhost:5432"
echo "pgAdmin:           http://localhost:8888  (login: ${PGADMIN_DEFAULT_EMAIL} / ${PGADMIN_DEFAULT_PASSWORD})"
echo "OpenSearch:        http://localhost:9200  (login: ${OPENSEARCH_USERNAME} / ${OPENSEARCH_PASSWORD})"
echo "Grafana:           http://localhost:3000  (login: ${GF_SECURITY_ADMIN_USER} / ${GF_SECURITY_ADMIN_PASSWORD})"
echo "Cerebro:           http://localhost:9000"
echo ""
echo "Use 'docker compose ps' to see status."
echo "Use 'docker compose logs SERVICE' for logs."
echo ""
echo "Done. Make sure your .env is filled with real values before starting!"
