package chongwm.utils.pdf.exception;

/**
 * Base exception for PDF splitter operations.
 */
public class PdfSplitterException extends Exception {
    
    public PdfSplitterException(String message) {
        super(message);
    }
    
    public PdfSplitterException(String message, Throwable cause) {
        super(message, cause);
    }
}
