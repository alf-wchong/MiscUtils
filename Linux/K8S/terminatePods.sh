#!/bin/bash

# Default values
NAMESPACE="yourAppNamespaceInYourK8SCluster"
VALIDATE=false

# Function to display usage
usage() {
    echo "Usage: $0 [--ns NAMESPACE] [--validate] <pod_name_part1> [pod_name_part2] ..."
    echo ""
    echo "Options:"
    echo "  --ns NAMESPACE    Specify the Kubernetes namespace (default: uidlys-dev)"
    echo "  --validate        Prompt for confirmation before deleting pods (default: false)"
    echo ""
    echo "Examples:"
    echo "  $0 web-app frontend"
    echo "  $0 --ns production api-service"
    echo "  $0 --validate --ns staging worker queue processor"
    echo "  $0 --validate web service"
    exit 1
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --ns)
            NAMESPACE="$2"
            shift 2
            ;;
        --validate)
            VALIDATE=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        *)
            # All remaining arguments are pod name parts
            break
            ;;
    esac
done

# Check if at least one pod name part is provided
if [[ $# -eq 0 ]]; then
    echo "Error: At least one pod name part must be provided."
    usage
fi

# Store all remaining arguments as pod name parts
POD_NAME_PARTS=("$@")

# Construct the full pod name by joining all parts with hyphens
POD_NAME=$(IFS='-'; echo "${POD_NAME_PARTS[*]}")

echo "Namespace: $NAMESPACE"
echo "Looking for pods matching: $POD_NAME"
if [[ "$VALIDATE" == true ]]; then
    echo "Validation: enabled"
else
    echo "Validation: disabled (automatic deletion)"
fi
echo ""

# Check if kubectl is available
if ! command -v kubectl &> /dev/null; then
    echo "Error: kubectl is not installed or not in PATH"
    exit 1
fi

# Check if the namespace exists
if ! kubectl get namespace "$NAMESPACE" &> /dev/null; then
    echo "Error: Namespace '$NAMESPACE' does not exist"
    exit 1
fi

# Find pods that match the pattern
MATCHING_PODS=$(kubectl get pods -n "$NAMESPACE" --no-headers -o custom-columns=":metadata.name" | grep "$POD_NAME" || true)

if [[ -z "$MATCHING_PODS" ]]; then
    echo "No pods found matching pattern: $POD_NAME"
    exit 0
fi

echo "Found the following matching pods:"
echo "$MATCHING_PODS"
echo ""

# Handle validation if enabled
PROCEED=true
if [[ "$VALIDATE" == true ]]; then
    read -p "Do you want to delete these pods? (y/N): " -n 1 -r
    echo ""
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        PROCEED=false
    fi
fi

if [[ "$PROCEED" == true ]]; then
    echo "Deleting pods..."
    DELETED_COUNT=0
    FAILED_COUNT=0
    
    while IFS= read -r pod; do
        if [[ -n "$pod" ]]; then
            echo "Deleting pod: $pod"
            kubectl delete pod "$pod" -n "$NAMESPACE"
            if [[ $? -eq 0 ]]; then
                echo "✓ Successfully deleted: $pod"
                ((DELETED_COUNT++))
            else
                echo "✗ Failed to delete: $pod"
                ((FAILED_COUNT++))
            fi
        fi
    done <<< "$MATCHING_PODS"
    
    echo ""
    echo "Summary:"
    echo "  Successfully deleted: $DELETED_COUNT pods"
    if [[ $FAILED_COUNT -gt 0 ]]; then
        echo "  Failed to delete: $FAILED_COUNT pods"
    fi
    echo "Done!"
else
    echo "Operation cancelled."
fi
