# Currency to Words Converter

A single Java class that converts currency values into their written word equivalents, similar to how amounts are written on cheques. Supports major world currencies with different formatting conventions.

## Installation

Download `CurrencyToWords.java` and add it to your project. That's it!

```bash
# Download the file
curl -O https://raw.githubusercontent.com/alf-wchong/MiscUtils/refs/heads/main/CurrencyToWords/currencyToWords.java

# Copy it to your project
cp CurrencyToWords.java /path/to/your/project/
```

## Usage
### Compile and Run

```bash
# Compile
javac CurrencyToWords.java

# Run the demo
java example.chong.currency.CurrencyToWords
```

### Use in Your Code

```java
import example.chongwm.currency.CurrencyToWords;
import example.chongwm.currency.CurrencyToWords.CurrencyResult;

public class MyApp {
    public static void main(String[] args) {
        CurrencyToWords converter = new CurrencyToWords();
        
        CurrencyResult result = converter.convert("$1,234.56");
        System.out.println(result.getWordsOutput());
        // Output: one thousand two hundred thirty four dollars and fifty six cents
    }
}
```

### Features

- 🌍 Supports 10+ major currencies (USD, EUR, GBP, JPY, CNY, INR, AUD, CAD, CHF, SEK)
- 🔢 Handles US format (1,234.56) and European format (1.234,56)
- 💰 Processes both major and minor units (dollars/cents, pounds/pence, etc.)
- 📈 Converts amounts up to 999 trillion
- ⚠️ Clear error codes and messages
- 📚 Well documented with JavaDoc
- 🚀 Zero dependencies - just one Java file!

### Examples

```bash
CurrencyToWords converter = new CurrencyToWords();

// US Dollar
converter.convert("$1,234.56");
// one thousand two hundred thirty four dollars and fifty six cents

// Euro (European format)
converter.convert("€1.234,56");
// one thousand two hundred thirty four euros and fifty six cents

// British Pound
converter.convert("£999.99");
// nine hundred ninety nine pounds and ninety nine pence

// Large amount
converter.convert("$1,000,000.00");
// one million dollars

// Error handling
CurrencyResult result = converter.convert("InvalidCurrency");
if (!result.isSuccess()) {
    System.out.println("Error: " + result.getErrorMessage());
}
```

### Supported Currencies
| Symbol | Currency | Example |
| :--- | :--- | :--- |
| $, USD | US Dollar | $1,234.56 |
| €, EUR | Euro | €1.234,56 |
| £, GBP | British Pound | £1,234.56 |
| ¥, JPY | Japanese Yen | ¥1,234 |
| CNY | Chinese Yuan | CNY 1,234.56 |
| ₹, INR | Indian Rupee | ₹1,234.56 |
| AUD | Australian Dollar | AUD 1,234.56 |
| CAD | Canadian Dollar | CAD 1,234.56 |
| CHF | Swiss Franc | CHF 1,234.56 |
| kr, SEK | Swedish Krona | 1.234,56 kr |

### Error Codes
| Code | Description |
| :--- | :--- |
| 0 | Success |
| 1 | Invalid format |
| 2 | Unknown currency |
| 3 | Negative value |
| 4 | Value too large (>999 trillion) |
| 5 | Invalid number |

### Requirements
- Java 8 or higher
- No external dependencies

### Quick Integration

If you're using a package structure:
```bash
# Create directory structure
mkdir -p example/chongwm/currency

# Move the file
mv CurrencyToWords.java example/chongwm/currency/

# Compile
javac example/chongwm/currency/CurrencyToWords.java

# Run
java example.chongwm.currency.CurrencyToWords
```

If you want to use it without packages, simply remove the package declaration at the top of the file:
```bash
Remove this line:
// package example.chongwm.currency;
```
That's it! No build tools, no dependencies, no complexity. Just download and use.
