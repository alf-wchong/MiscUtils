# Kubernetes Pod Deletion Script

Bash script to **search for and delete Kubernetes pods** matching a given name pattern within a specified namespace. It also supports optional validation prompts before deletion.

---

## Features

- Delete one or more pods based on partial name matches.  
- Supports **namespace selection**.  
- Optional **confirmation prompt** before deletion.  
- Provides a summary of successful and failed deletions.  
- Gracefully handles missing `kubectl`, invalid namespaces, and no matching pods.  

---

## Usage

``` bash
./delete-pods.sh [--ns NAMESPACE] [--validate] <pod_name_part1> [pod_name_part2] ...
```


### Options

- `--ns NAMESPACE`  
  Specify the Kubernetes namespace (default: `yourAppNamespaceInYourK8SCluster`).

- `--validate`  
  Enable confirmation prompt before deleting pods. If not set, deletion happens automatically.

- `-h`, `--help`  
  Show usage information.

### Examples

Delete pods matching "web-app" in the default namespace
```
./delete-pods.sh web-app
```

Delete pods matching "frontend" in namespace "production"
```
./delete-pods.sh --ns production frontend
```

Delete multiple related pods in staging (joined as worker-queue-processor)
```
./delete-pods.sh --ns staging worker-queue-processor
```

Prompt for confirmation before deleting pods named "web" and "service"
```
./delete-pods.sh --validate web service
```


---

## Output Example

Namespace: production
Looking for pods matching: api-service
Validation: enabled

Found the following matching pods:
api-service-6f7cd88999-tkhnw
api-service-6f7cd88999-wmfsl

Do you want to delete these pods? (y/N): y
Deleting pods...
Deleting pod: api-service-6f7cd88999-tkhnw
✓ Successfully deleted: api-service-6f7cd88999-tkhnw
Deleting pod: api-service-6f7cd88999-wmfsl
✓ Successfully deleted: api-service-6f7cd88999-wmfsl

Summary:
Successfully deleted: 2 pods
Done!


---

## Requirements

- **kubectl** installed and configured to access your Kubernetes cluster.  
- Sufficient permissions to delete pods in the target namespace.  

---

## Installation

1. Clone this repository or save the script as `delete-pods.sh`.
2. Make it executable:
```
chmod +x delete-pods.sh
```

3. Run it from your terminal.

---

## Notes

- The script uses simple string matching with `grep`. Ensure your pod name patterns are specific enough to avoid deleting unintended pods.  
- Use in development, staging Kubernetes clusters. Use with caution.  
