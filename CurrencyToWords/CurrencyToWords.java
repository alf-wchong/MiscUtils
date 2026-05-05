package example.chongwm.currency;

import java.math.BigDecimal;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * CurrencyToWords converts currency values with symbols into their word equivalents,
 * similar to how amounts are written on cheques.
 * 
 * <p>Supports major world currencies including USD, EUR, GBP, JPY, CNY, INR, AUD, CAD, CHF, and more.
 * Handles various formatting conventions (comma vs. dot separators) and converts amounts
 * up to trillions into their written form.</p>
 * 
 * <p>Example usage:</p>
 * <pre>
 * CurrencyToWords converter = new CurrencyToWords();
 * CurrencyResult result = converter.convert("$1,234.56");
 * System.out.println(result.getWordsOutput());
 * // Output: "one thousand two hundred thirty four dollars and fifty six cents"
 * </pre>
 * 
 * @author chongwm
 * @version 1.0
 */
public class CurrencyToWords {
    
    /**
     * Result object containing the converted text, return code, and any error messages.
     */
    public static class CurrencyResult {
        private final String wordsOutput;
        private final int returnCode;
        private final String errorMessage;
        
        /**
         * Constructs a successful result.
         * 
         * @param wordsOutput the currency amount in words
         */
        public CurrencyResult(String wordsOutput) {
            this.wordsOutput = wordsOutput;
            this.returnCode = 0;
            this.errorMessage = null;
        }
        
        /**
         * Constructs an error result.
         * 
         * @param returnCode the error code (non-zero)
         * @param errorMessage description of the error
         */
        public CurrencyResult(int returnCode, String errorMessage) {
            this.wordsOutput = "ERROR: " + errorMessage;
            this.returnCode = returnCode;
            this.errorMessage = errorMessage;
        }
        
        public String getWordsOutput() { return wordsOutput; }
        public int getReturnCode() { return returnCode; }
        public String getErrorMessage() { return errorMessage; }
        public boolean isSuccess() { return returnCode == 0; }
    }
    
    /**
     * Custom exception for currency conversion errors.
     */
    public static class CurrencyConversionException extends Exception {
        private final int errorCode;
        
        /**
         * Constructs a new exception with the specified error code and message.
         * 
         * @param errorCode numeric error code
         * @param message error description
         */
        public CurrencyConversionException(int errorCode, String message) {
            super(message);
            this.errorCode = errorCode;
        }
        
        public int getErrorCode() { return errorCode; }
    }
    
    // Error codes for different exception types
    public static final int ERROR_INVALID_FORMAT = 1;
    public static final int ERROR_UNKNOWN_CURRENCY = 2;
    public static final int ERROR_NEGATIVE_VALUE = 3;
    public static final int ERROR_VALUE_TOO_LARGE = 4;
    public static final int ERROR_INVALID_NUMBER = 5;
    
    /**
     * Internal class to hold currency configuration details.
     */
    private static class CurrencyInfo {
        String symbol;
        String majorUnit;      // e.g., "dollar", "euro"
        String minorUnit;      // e.g., "cent", "penny"
        boolean usesCommaDecimal;  // true if uses comma as decimal separator (European style)
        int minorUnitDivisor;  // usually 100 for most currencies
        
        CurrencyInfo(String symbol, String majorUnit, String minorUnit, 
                     boolean usesCommaDecimal, int minorUnitDivisor) {
            this.symbol = symbol;
            this.majorUnit = majorUnit;
            this.minorUnit = minorUnit;
            this.usesCommaDecimal = usesCommaDecimal;
            this.minorUnitDivisor = minorUnitDivisor;
        }
    }
    
    // Map of currency symbols to their configurations
    private final Map<String, CurrencyInfo> currencyMap;
    
    // Arrays for converting numbers to words
    private static final String[] ONES = {
        "", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
        "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen",
        "seventeen", "eighteen", "nineteen"
    };
    
    private static final String[] TENS = {
        "", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"
    };
    
    private static final String[] THOUSANDS = {
        "", "thousand", "million", "billion", "trillion"
    };
    
    /**
     * Constructs a new CurrencyToWords converter with support for major world currencies.
     */
    public CurrencyToWords() {
        currencyMap = new HashMap<>();
        
        // North American currencies
        currencyMap.put("$", new CurrencyInfo("$", "dollar", "cent", false, 100));
        currencyMap.put("USD", new CurrencyInfo("USD", "dollar", "cent", false, 100));
        currencyMap.put("CAD", new CurrencyInfo("CAD", "dollar", "cent", false, 100));
        
        // European currencies
        currencyMap.put("€", new CurrencyInfo("€", "euro", "cent", true, 100));
        currencyMap.put("EUR", new CurrencyInfo("EUR", "euro", "cent", true, 100));
        
        // British currency
        currencyMap.put("£", new CurrencyInfo("£", "pound", "penny", false, 100));
        currencyMap.put("GBP", new CurrencyInfo("GBP", "pound", "penny", false, 100));
        
        // Asian currencies
        currencyMap.put("¥", new CurrencyInfo("¥", "yen", "sen", false, 100));
        currencyMap.put("JPY", new CurrencyInfo("JPY", "yen", "sen", false, 100));
        currencyMap.put("CNY", new CurrencyInfo("CNY", "yuan", "fen", false, 100));
        currencyMap.put("₹", new CurrencyInfo("₹", "rupee", "paisa", false, 100));
        currencyMap.put("INR", new CurrencyInfo("INR", "rupee", "paisa", false, 100));
        
        // Other major currencies
        currencyMap.put("AUD", new CurrencyInfo("AUD", "dollar", "cent", false, 100));
        currencyMap.put("CHF", new CurrencyInfo("CHF", "franc", "centime", false, 100));
        currencyMap.put("kr", new CurrencyInfo("kr", "krona", "öre", true, 100));
        currencyMap.put("SEK", new CurrencyInfo("SEK", "krona", "öre", true, 100));
    }
    
    /**
     * Converts a currency string to its word equivalent.
     * 
     * @param input currency string (e.g., "$1,234.56", "€1.234,56")
     * @return CurrencyResult object containing the conversion result and status
     */
    public CurrencyResult convert(String input) {
        try {
            // Step 1: Validate and parse the input
            if (input == null || input.trim().isEmpty()) {
                throw new CurrencyConversionException(ERROR_INVALID_FORMAT, 
                    "Input cannot be null or empty");
            }
            
            input = input.trim();
            
            // Step 2: Extract currency symbol from the input
            CurrencyInfo currencyInfo = extractCurrency(input);
            
            // Step 3: Extract and normalize the numeric value
            String numericPart = extractNumericPart(input, currencyInfo.symbol);
            
            // Step 4: Parse the value considering the currency's decimal format
            BigDecimal value = parseValue(numericPart, currencyInfo);
            
            // Step 5: Validate the value
            validateValue(value);
            
            // Step 6: Convert to words
            String words = convertToWords(value, currencyInfo);
            
            return new CurrencyResult(words);
            
        } catch (CurrencyConversionException e) {
            return new CurrencyResult(e.getErrorCode(), e.getMessage());
        }
    }
    
    /**
     * Extracts and identifies the currency from the input string.
     * 
     * @param input the currency string
     * @return CurrencyInfo object for the identified currency
     * @throws CurrencyConversionException if currency cannot be identified
     */
    private CurrencyInfo extractCurrency(String input) throws CurrencyConversionException {
        // Try to match currency symbols at the beginning or end
        for (Map.Entry<String, CurrencyInfo> entry : currencyMap.entrySet()) {
            String symbol = entry.getKey();
            if (input.startsWith(symbol) || input.endsWith(symbol) || 
                input.toUpperCase().contains(symbol.toUpperCase())) {
                return entry.getValue();
            }
        }
        
        throw new CurrencyConversionException(ERROR_UNKNOWN_CURRENCY, 
            "Unknown or unsupported currency symbol in input: " + input);
    }
    
    /**
     * Extracts the numeric portion from the currency string, removing the currency symbol.
     * 
     * @param input the full currency string
     * @param symbol the currency symbol to remove
     * @return the numeric portion as a string
     */
    private String extractNumericPart(String input, String symbol) {
        // Remove currency symbol and any spaces
        String numeric = input.replace(symbol, "").trim();
        
        // Remove any remaining currency codes (like USD, EUR, etc.)
        for (String code : currencyMap.keySet()) {
            numeric = numeric.replace(code, "").trim();
        }
        
        return numeric;
    }
    
    /**
     * Parses the numeric string into a BigDecimal, handling different decimal separator conventions.
     * 
     * @param numericPart the numeric string (may contain separators)
     * @param currencyInfo configuration for this currency
     * @return BigDecimal representation of the value
     * @throws CurrencyConversionException if the number format is invalid
     */
    private BigDecimal parseValue(String numericPart, CurrencyInfo currencyInfo) 
            throws CurrencyConversionException {
        try {
            // Normalize the number format
            String normalized;
            
            if (currencyInfo.usesCommaDecimal) {
                // European format: 1.234.567,89
                // Remove dots (thousands separator), replace comma with dot
                normalized = numericPart.replace(".", "").replace(",", ".");
            } else {
                // US/UK format: 1,234,567.89
                // Remove commas (thousands separator)
                normalized = numericPart.replace(",", "");
            }
            
            // Additional validation: check for multiple decimal points
            if (normalized.chars().filter(ch -> ch == '.').count() > 1) {
                throw new CurrencyConversionException(ERROR_INVALID_NUMBER,
                    "Invalid number format: multiple decimal separators found");
            }
            
            return new BigDecimal(normalized);
            
        } catch (NumberFormatException e) {
            throw new CurrencyConversionException(ERROR_INVALID_NUMBER,
                "Invalid number format: " + numericPart);
        }
    }
    
    /**
     * Validates that the currency value is within acceptable ranges.
     * 
     * @param value the value to validate
     * @throws CurrencyConversionException if the value is invalid
     */
    private void validateValue(BigDecimal value) throws CurrencyConversionException {
        // Check for negative values
        if (value.compareTo(BigDecimal.ZERO) < 0) {
            throw new CurrencyConversionException(ERROR_NEGATIVE_VALUE,
                "Negative currency values are not supported");
        }
        
        // Check for values exceeding trillion
        BigDecimal maxValue = new BigDecimal("999999999999999.99"); // Just under quadrillion
        if (value.compareTo(maxValue) > 0) {
            throw new CurrencyConversionException(ERROR_VALUE_TOO_LARGE,
                "Value exceeds maximum supported amount (999 trillion)");
        }
    }
    
    /**
     * Converts a BigDecimal currency value to its word representation.
     * 
     * @param value the currency value
     * @param currencyInfo configuration for this currency
     * @return the value in words
     */
    private String convertToWords(BigDecimal value, CurrencyInfo currencyInfo) {
        // Split into major and minor units (e.g., dollars and cents)
        long majorUnits = value.longValue();
        int minorUnits = value.remainder(BigDecimal.ONE)
                             .multiply(new BigDecimal(currencyInfo.minorUnitDivisor))
                             .intValue();
        
        StringBuilder result = new StringBuilder();
        
        // Convert major units (e.g., dollars)
        if (majorUnits == 0) {
            result.append("zero");
        } else {
            result.append(convertNumberToWords(majorUnits));
        }
        
        // Add major unit name with proper pluralization
        result.append(" ");
        result.append(majorUnits == 1 ? currencyInfo.majorUnit : currencyInfo.majorUnit + "s");
        
        // Convert minor units (e.g., cents) if present
        if (minorUnits > 0) {
            result.append(" and ");
            result.append(convertNumberToWords(minorUnits));
            result.append(" ");
            result.append(minorUnits == 1 ? currencyInfo.minorUnit : 
                         getMinorUnitPlural(currencyInfo.minorUnit));
        }
        
        return result.toString();
    }
    
    /**
     * Gets the plural form of minor currency units (handles special cases like penny/pence).
     * 
     * @param minorUnit the singular form
     * @return the plural form
     */
    private String getMinorUnitPlural(String minorUnit) {
        // Special case for British pence
        if (minorUnit.equals("penny")) {
            return "pence";
        }
        // Default: just add 's'
        return minorUnit + "s";
    }
    
    /**
     * Converts a long number to its word representation.
     * Handles numbers up to trillions.
     * 
     * @param number the number to convert
     * @return the number in words
     */
    private String convertNumberToWords(long number) {
        if (number == 0) {
            return "zero";
        }
        
        StringBuilder words = new StringBuilder();
        int thousandCounter = 0;
        
        // Process the number in groups of three digits (thousands, millions, billions, etc.)
        while (number > 0) {
            // Get the last three digits
            int chunk = (int)(number % 1000);
            
            if (chunk != 0) {
                String chunkWords = convertChunkToWords(chunk);
                
                // Add the scale word (thousand, million, billion, trillion)
                if (thousandCounter > 0) {
                    chunkWords += " " + THOUSANDS[thousandCounter];
                }
                
                // Prepend to the result (we're processing right to left)
                if (words.length() > 0) {
                    words.insert(0, chunkWords + " ");
                } else {
                    words.insert(0, chunkWords);
                }
            }
            
            // Move to the next group of three digits
            number /= 1000;
            thousandCounter++;
        }
        
        return words.toString().trim();
    }
    
    /**
     * Converts a three-digit number (0-999) to words.
     * 
     * @param number the three-digit number
     * @return the number in words
     */
    private String convertChunkToWords(int number) {
        StringBuilder chunk = new StringBuilder();
        
        // Handle hundreds place
        int hundreds = number / 100;
        if (hundreds > 0) {
            chunk.append(ONES[hundreds]).append(" hundred");
            number %= 100;
            if (number > 0) {
                chunk.append(" ");
            }
        }
        
        // Handle tens and ones places
        if (number >= 20) {
            int tens = number / 10;
            chunk.append(TENS[tens]);
            number %= 10;
            if (number > 0) {
                chunk.append(" ").append(ONES[number]);
            }
        } else if (number > 0) {
            chunk.append(ONES[number]);
        }
        
        return chunk.toString();
    }
    
    /**
     * Capitalizes the first letter of each word and lowercases all other letters.
     * 
     * @param input the string to capitalize
     * @return the capitalized string
     */
    static protected String capitalizeWords(String input) {
        if (input == null || input.isEmpty()) {
            return input;
        }
        
        String[] words = input.split("\\s+");
        StringBuilder result = new StringBuilder();
        
        for (String word : words) {
            if (word.length() > 0) {
                result.append(Character.toUpperCase(word.charAt(0)))
                      .append(word.substring(1).toLowerCase())
                      .append(" ");
            }
        }
        
        return result.toString().trim();
    }    
    
    
    
    /**
     * Main method demonstrating usage of the CurrencyToWords converter.
     * 
     * @param args command line arguments (not used)
     */
    public static void main(String[] args) 
    {
        CurrencyToWords converter = new CurrencyToWords();
        if (args.length<1)
        // Test cases demonstrating various currencies and formats
        {String[] testCases = {
            "$1,234.56",           // US format
            "€1.234,56",           // European format
            "£999.99",             // British pounds
            "¥1000",               // Japanese yen (no decimals typically)
            "$1,000,000.00",       // One million dollars
            "€0,50",               // Fifty cents
            "$0.01",               // One cent
            "InvalidCurrency123",  // Error case
            "-$50.00",             // Negative (error case)
            "$1,234,567,890.12"    // Large amount
        };
        args = testCases;
        System.out.println("Currency to Words Converter - Test Results\n");
        System.out.println("=".repeat(80));

        }
        
        for (String amt : args) {
            CurrencyResult result = converter.convert(amt);
            System.out.println("\nInput:       " + amt);
            System.out.println("Output:      " + capitalizeWords(result.getWordsOutput()));
            System.out.println("Return Code: " + result.getReturnCode());
            if (!result.isSuccess()) {
                System.out.println("Error:       " + result.getErrorMessage());
            }
            System.out.println("-".repeat(80));
        }
    }
}
