# Maven Builder (Fedora + Azul Zulu 17, RAM-backed)

A lightweight, **throwaway development container** for building Java/Maven projects in a clean, reproducible environment.

It is designed for:

* Fast, **ephemeral builds**
* Zero pollution of your host system
* Compatibility with systems requiring **Java 17 (Azul Zulu)** such as Doxis CSB
* High-performance builds using **RAM-backed storage**

---

## Why this image exists

Modern Java builds tend to accumulate:

* Local Maven repositories (`~/.m2`)
* Temporary build artifacts
* Tooling inconsistencies across machines

This container solves those problems by providing:

### 1. Clean, disposable environment

Each container starts from scratch and is deleted after use.

### 2. Vendor-aligned JVM

Uses **Azul Zulu OpenJDK 17**, ensuring compatibility with systems that require:

* consistent JVM vendor
* Java 17 runtime

### 3. RAM-backed builds

Both:

* Maven repository (`~/.m2`)
* Project workspace

are stored in memory (`tmpfs`), not on disk.

This results in:

* faster builds
* no SSD wear
* automatic cleanup on container stop

### 4. Comfortable developer tooling

Includes:

* Maven
* Git
* Vim
* Tree
* Less
* Basic Linux utilities

This is not a minimal runtime image—it is a **developer workspace**.

---

## What’s inside the image

* Fedora 43 base
* Azul Zulu OpenJDK 17
* Maven
* Common CLI tools for development and debugging

---

## Build the image

From the directory containing the Dockerfile:

```bash
docker build -t maven-builder:zulu17 .
```

---

## Run the container (ephemeral mode)

Start the container in the background:

```bash
docker run -d --rm \
  --name maven-builder \
  --tmpfs /root/.m2:rw,exec,nosuid,nodev,size=1536m \
  --tmpfs /workspace:rw,exec,nosuid,nodev,size=3g \
  maven-builder:zulu17 \
  sleep infinity
```

### What this does

* `--rm` → container is deleted when stopped
* `--tmpfs` → stores data in RAM instead of disk
* `sleep infinity` → keeps container running for interactive use

---

## Enter the container

```bash
docker exec -it maven-builder bash
```

You are now inside a clean Linux environment.

---

## Typical workflow

Inside the container:

```bash
cd /workspace
git clone <your-repository>
cd <your-project>

mvn clean verify
```

---

## Verify Java version

```bash
java -version
mvn -version
```

You should see:

* Java 17
* Azul Zulu distribution

---

## Stopping the container

```bash
docker stop maven-builder
```

This will:

* stop the container
* delete it automatically (`--rm`)
* erase all build artifacts and dependencies

---

## Key concepts (for non-container users)

### What is a container?

A container is a **lightweight, isolated environment** that runs on your machine but behaves like a separate system.

It does **not** require a virtual machine.

---

### What does “ephemeral” mean?

It means:

* nothing is saved permanently
* everything is discarded when the container stops

This ensures:

* no leftover files
* no dependency conflicts
* fully repeatable builds

---

### Why use RAM (`tmpfs`)?

Instead of writing to disk:

* files are stored in memory
* access is faster
* everything disappears automatically

---

## When to use this

Use this image when you want:

* a clean build environment
* guaranteed Java 17 compatibility
* no local setup or pollution
* fast, disposable builds

---

## When NOT to use this

This is not intended for:

* running production applications
* long-lived services
* persistent development environments

---

## Summary

This image provides:

* Fedora-based development environment
* Azul Zulu Java 17
* Maven + essential tools
* RAM-backed, throwaway builds

It behaves like a **temporary VM for Java builds**, without the overhead of a full virtual machine.

---
