#!/bin/bash
set -euo pipefail

docker compose -p glory build
docker compose -p glory up -d --remove-orphans
