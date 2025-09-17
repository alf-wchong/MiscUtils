# PDF Splitter

A Java application that splits PDF files into individual documents based on a JSON configuration.

## Features

- Split PDFs based on page ranges defined in JSON configuration
- Safe filename generation with duplicate handling
- Cross-platform compatibility (Windows/Linux)
- Comprehensive error handling and validation
- Logging support
- Unit tests with high coverage

## Requirements

- Java 11 or higher
- Maven 3.6+

## Building

```bash
mvn clean compile
mvn test
mvn package
```

## Usage

### Command Line
```bash
java -jar target/pdf-splitter-1.0.0.jar input.pdf config.json output/
```

### Configuration Format

Create a JSON file with the following structure:

```json
{
  "documents": [
    {
      "category": "invoice",
      "start_page": 1,
      "end_page": 2,
      "confidence": 95
    },
    {
      "category": "receipt", 
      "start_page": 3,
      "end_page": 4,
      "confidence": 88
    }
  ]
}
```

### Output Files

Files are named using the pattern: `{category}_{confidence}.pdf`

For duplicates, an index is added: `{category}_{confidence}_1.pdf`

## Error Handling

The application validates:
- File existence and permissions
- JSON configuration syntax and schema
- Page range validity
- No overlapping page ranges
- Page numbers within document bounds

## Logging

Logs are written to:
- Console (INFO level)
- File: `pdf-splitter.log` (INFO level)

## Testing

Run tests with:
```bash
mvn test
```

## Project Structure

```
pdf-splitter/
├── pom.xml
├── src/
│   ├── main/
│   │   ├── java/
│   │   │   └── chongwm/
│   │   │       └── utils/
│   │   │           └── pdf/
│   │   │               ├── PdfSplitterApplication.java
│   │   │               ├── model/
│   │   │               ├── service/
│   │   │               ├── util/
│   │   │               └── exception/
│   │   └── resources/
│   │       └── logback.xml
│   └── test/
│       ├── java/
│       └── resources/
└── README.md
```

## License

MIT License