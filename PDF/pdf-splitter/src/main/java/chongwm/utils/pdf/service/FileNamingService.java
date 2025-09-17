package chongwm.utils.pdf.service;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * Service for generating safe file names with duplicate handling.
 */
public class FileNamingService {
    
    private static final Logger logger = LoggerFactory.getLogger(FileNamingService.class);
    
    // Pattern for invalid filename characters on Windows and Linux
    private static final Pattern INVALID_CHARS = Pattern.compile("[<>:\"/\\\\|?*\\x00-\\x1F]");
    private static final String REPLACEMENT_CHAR = "_";
    
    private final Map<String, Integer> fileNameCounts = new HashMap<>();

    /**
     * Generates a safe, unique filename for the given category and confidence.
     * 
     * @param category the document category
     * @param confidence the confidence level
     * @param outputDirectory the target output directory
     * @return a safe, unique filename
     */
    public String generateUniqueFileName(String category, int confidence, String outputDirectory) {
        String sanitizedCategory = sanitizeFileName(category);
        String baseFileName = String.format("%s_%d.pdf", sanitizedCategory, confidence);
        
        String uniqueFileName = ensureUniqueFileName(baseFileName, outputDirectory);
        
        logger.debug("Generated filename: {} for category: {}, confidence: {}", 
                    uniqueFileName, category, confidence);
        return uniqueFileName;
    }

    /**
     * Sanitizes a filename by removing or replacing invalid characters.
     * 
     * @param fileName the original filename
     * @return sanitized filename
     */
    private String sanitizeFileName(String fileName) {
        if (fileName == null || fileName.trim().isEmpty()) {
            return "unknown";
        }
        
        // Replace invalid characters
        String sanitized = INVALID_CHARS.matcher(fileName.trim()).replaceAll(REPLACEMENT_CHAR);
        
        // Handle reserved Windows filenames
        if (isReservedWindowsName(sanitized)) {
            sanitized = "file_" + sanitized;
        }
        
        // Ensure filename is not too long (255 chars is typical filesystem limit)
        if (sanitized.length() > 200) { // Leave room for confidence, extension, and index
            sanitized = sanitized.substring(0, 200);
        }
        
        // Remove trailing dots and spaces (problematic on Windows)
        sanitized = sanitized.replaceAll("[.\\s]+$", "");
        
        // Ensure it's not empty after sanitization
        if (sanitized.isEmpty()) {
            sanitized = "document";
        }
        
        return sanitized;
    }

    /**
     * Checks if a filename is a reserved Windows name.
     * 
     * @param fileName the filename to check
     * @return true if it's a reserved name
     */
    private boolean isReservedWindowsName(String fileName) {
        String upperName = fileName.toUpperCase();
        String[] reserved = {"CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", 
                           "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", 
                           "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"};
        
        for (String reservedName : reserved) {
            if (upperName.equals(reservedName) || upperName.startsWith(reservedName + ".")) {
                return true;
            }
        }
        return false;
    }

    /**
     * Ensures the filename is unique in the target directory by adding an index if necessary.
     * 
     * @param baseFileName the base filename
     * @param outputDirectory the target directory
     * @return unique filename
     */
    private String ensureUniqueFileName(String baseFileName, String outputDirectory) {
        String key = outputDirectory + File.separator + baseFileName;
        
        if (!fileExists(outputDirectory, baseFileName) && !fileNameCounts.containsKey(key)) {
            fileNameCounts.put(key, 1);
            return baseFileName;
        }
        
        int count = fileNameCounts.getOrDefault(key, 1);
        String fileName;
        String baseName = baseFileName.substring(0, baseFileName.lastIndexOf('.'));
        String extension = baseFileName.substring(baseFileName.lastIndexOf('.'));
        
        do {
            fileName = String.format("%s_%d%s", baseName, count, extension);
            count++;
        } while (fileExists(outputDirectory, fileName) || 
                fileNameCounts.containsKey(outputDirectory + File.separator + fileName));
        
        fileNameCounts.put(key, count);
        return fileName;
    }

    /**
     * Checks if a file exists in the given directory.
     * 
     * @param directory the directory to check
     * @param fileName the filename to check
     * @return true if the file exists
     */
    private boolean fileExists(String directory, String fileName) {
        File file = new File(directory, fileName);
        return file.exists();
    }

    /**
     * Resets the internal file name counter. Useful for testing or when processing new batches.
     */
    public void reset() {
        fileNameCounts.clear();
    }
}