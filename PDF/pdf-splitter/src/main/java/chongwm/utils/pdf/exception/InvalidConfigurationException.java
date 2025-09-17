package chongwm.utils.pdf.exception;

/**
 * Exception thrown when configuration is invalid.
 */
public class InvalidConfigurationException extends PdfSplitterException {
    
    public InvalidConfigurationException(String message) {
        super(message);
    }
    
    public InvalidConfigurationException(String message, Throwable cause) {
        super(message, cause);
    }
}