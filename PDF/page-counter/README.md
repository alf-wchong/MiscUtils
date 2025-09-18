# PDF Page Counter

A Java 11 Maven application that counts pages in PDF files and compares the count against JSON document definitions.

## Features

- Counts pages in PDF files using Apache PDFBox
- Parses JSON document definitions with page ranges
- Compares PDF page count with sum of JSON page ranges
- Detects overlapping page ranges in JSON
- Supports console and file output modes
- Comprehensive error handling with specific exit codes

## Build

```bash
mvn clean package
```

This creates an executable JAR with all dependencies: `target/counter-1.0.0.jar`

## Usage

```bash
java -jar target/counter-1.0.0.jar <pdf_file> <json_file> [-o <output_mode>]
```

### Parameters

- `pdf_file`: Path to the PDF file to analyze
- `json_file`: Path to the JSON file containing document definitions
- `-o <output_mode>`: Optional output mode
  - `console` (default): Output result to console
  - `f`: Output result to file named `<pdf_file>.result`

### Examples

```bash
# Output to console
java -jar target/counter-1.0.0.jar document.pdf definitions.json

# Output to file
java -jar target/counter-1.0.0.jar document.pdf definitions.json -o f
```

## JSON Schema

The JSON file must follow this [schema](https://github.com/alf-wchong/MiscUtils/tree/main/PDF/pdf-splitter#json-schema-for-pdf-structure):

```json
{
  "documents": [
    {
      "category": "invoice",
      "start_page": 1,
      "end_page": 3,
      "confidence": 95,
      "incomplete": false,
      "template": false
    },
    {
      "category": "receipt", 
      "start_page": 4,
      "end_page": 5,
      "confidence": 90
    }
  ]
}
```

### Required Fields
- `category`: Document classification
- `start_page`: Starting page number (minimum 1)
- `end_page`: Ending page number (minimum 1)  
- `confidence`: Confidence percentage (0-100)

### Optional Fields
- `incomplete`: Whether document is incomplete
- `template`: Whether document is a template

## Output Format

The application outputs the difference between PDF page count and JSON page sum:

- `+5`: PDF has 5 more pages than JSON defines
- `-3`: PDF has 3 fewer pages than JSON defines  
- `0`: PDF and JSON page counts match

## Error Codes

| Code | Description |
|------|-------------|
|  0   | No error encountered |
| -1   | Invalid command line arguments |
| -2   | Overlapping page ranges detected in JSON |
| -3   | PDF file not found |
| -4   | JSON file not found |
| -5   | Invalid or corrupted PDF file |
| -6   | Invalid JSON format or structure |
| -7   | General I/O error |
| -8   | Invalid page range (start_page > end_page or start_page < 1) |
| -9   | Error writing output file |
| -99  | Unexpected error |

<u>**Important Note:**</u> 

These are system exit codes that are returned to the operating system when the application encounters an error. These codes do not appear in the console output or result files - they are used by scripts, batch files, or other programs to determine if the application completed successfully or failed. 

Always check the numeric error code, either programmatically or on the console (e.g., using `echo $?` on Linux/Mac or `echo %ERRORLEVEL%` on Windows) after running the application.

A successful run returns exit code `0`. Any non-zero exit code indicates an error occurred.



## Page Range Validation

The application validates that:

1. `start_page` ≤ `end_page` for each document
2. `start_page` ≥ 1 for each document
3. No overlapping page ranges between documents

Overlapping ranges result in error code -2.

## Dependencies

- **Apache PDFBox 3.0.0**: PDF processing
- **Jackson 2.15.2**: JSON parsing
- **SLF4J 2.0.7 + Logback 1.4.11**: Logging
- **Apache Commons IO 2.13.0**: File utilities
- **JUnit 5.10.0**: Testing
- **Mockito 5.5.0**: Test mocking

## Development

### Running Tests

```bash
mvn test
```

### Project Structure

```
src/
├── main/java/chongwm/utils/pdf/counter/
│   ├── PdfPageCounter.java      # Main application class
│   ├── Document.java            # Document model class
│   ├── DocumentList.java        # Document list wrapper
│   └── PdfCounterException.java # Custom exception class
├── main/resources/
│   └── logback.xml             # Logging configuration
└── test/java/chongwm/utils/pdf/counter/
    └── PdfPageCounterTest.java # Unit tests
```

## Logging

The application logs to both console and file (`pdf-counter.log`). Log level can be adjusted in `src/main/resources/logback.xml`.

## Requirements

- Java 11 or higher
- Maven 3.6 or higher
