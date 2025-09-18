package chongwm.utils.pdf.counter;

/**
 * Custom exceptions for PDF counter application
 */
public class PdfCounterException extends Exception {
    private final int errorCode;

    public PdfCounterException(String message, int errorCode) {
        super(message);
        this.errorCode = errorCode;
    }

    public PdfCounterException(String message, int errorCode, Throwable cause) {
        super(message, cause);
        this.errorCode = errorCode;
    }

    public int getErrorCode() {
        return errorCode;
    }

    // Error code constants
    public static final int INVALID_ARGUMENTS = -1;
    public static final int OVERLAPPING_PAGE_RANGES = -2;
    public static final int PDF_FILE_NOT_FOUND = -3;
    public static final int JSON_FILE_NOT_FOUND = -4;
    public static final int INVALID_PDF = -5;
    public static final int INVALID_JSON = -6;
    public static final int IO_ERROR = -7;
    public static final int INVALID_PAGE_RANGE = -8;
    public static final int OUTPUT_FILE_ERROR = -9;
}