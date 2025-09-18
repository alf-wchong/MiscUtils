#!/bin/bash

# Build and run script for PDF Page Counter

set -e

echo "Building PDF Page Counter..."
mvn clean package -q

echo "Build complete. JAR location: target/counter-1.0.0.jar"
echo ""
echo "Usage examples:"
echo "  java -jar target/counter-1.0.0.jar document.pdf definitions.json"
echo "  java -jar target/counter-1.0.0.jar document.pdf definitions.json -o f"
echo ""

if [ "$1" = "run" ] && [ -n "$2" ] && [ -n "$3" ]; then
    echo "Running with provided arguments..."
    java -jar target/counter-1.0.0.jar "$2" "$3" "${@:4}"
elif [ "$1" = "test" ]; then
    echo "Running tests..."
    mvn test
else
    echo "To run the application:"
    echo "  ./build.sh run <pdf_file> <json_file> [options]"
    echo ""
    echo "To run tests:"
    echo "  ./build.sh test"
fi