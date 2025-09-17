package chongwm.utils.pdf.service;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.*;

class FileNamingServiceTest {

    private FileNamingService fileNamingService;

    @TempDir
    Path tempDir;

    @BeforeEach
    void setUp() {
        fileNamingService = new FileNamingService();
    }

    @Test
    void testGenerateUniqueFileName() {
        String fileName = fileNamingService.generateUniqueFileName("invoice", 95, tempDir.toString());
        assertEquals("invoice_95.pdf", fileName);
    }

    @Test
    void testGenerateUniqueFileNameWithSpecialCharacters() {
        String fileName = fileNamingService.generateUniqueFileName("invoice/receipt:test", 95, tempDir.toString());
        assertEquals("invoice_receipt_test_95.pdf", fileName);
    }

    @Test
    void testGenerateUniqueFileNameWithDuplicates() throws IOException {
        // Create an existing file
        File existingFile = new File(tempDir.toFile(), "invoice_95.pdf");
        existingFile.createNewFile();

        String fileName = fileNamingService.generateUniqueFileName("invoice", 95, tempDir.toString());
        assertEquals("invoice_95_1.pdf", fileName);
    }

    @Test
    void testGenerateUniqueFileNameWithMultipleDuplicates() {
        // Generate multiple files with same category and confidence
        String fileName1 = fileNamingService.generateUniqueFileName("invoice", 95, tempDir.toString());
        String fileName2 = fileNamingService.generateUniqueFileName("invoice", 95, tempDir.toString());
        String fileName3 = fileNamingService.generateUniqueFileName("invoice", 95, tempDir.toString());

        assertEquals("invoice_95.pdf", fileName1);
        assertEquals("invoice_95_1.pdf", fileName2);
        assertEquals("invoice_95_2.pdf", fileName3);
    }

    @Test
    void testGenerateUniqueFileNameWithReservedWindowsName() {
        String fileName = fileNamingService.generateUniqueFileName("CON", 95, tempDir.toString());
        assertEquals("file_CON_95.pdf", fileName);
    }

    @Test
    void testGenerateUniqueFileNameWithEmptyCategory() {
        String fileName = fileNamingService.generateUniqueFileName("", 95, tempDir.toString());
        assertEquals("unknown_95.pdf", fileName);
    }

    @Test
    void testGenerateUniqueFileNameWithLongCategory() {
        String longCategory = "a".repeat(250);
        String fileName = fileNamingService.generateUniqueFileName(longCategory, 95, tempDir.toString());
        
        // Should be truncated but still valid
        assertTrue(fileName.endsWith("_95.pdf"));
        assertTrue(fileName.length() < 255);
    }

    @Test
    void testReset() {
        // Generate a filename
        String fileName1 = fileNamingService.generateUniqueFileName("invoice", 95, tempDir.toString());
        assertEquals("invoice_95.pdf", fileName1);

        // Generate another with same parameters
        String fileName2 = fileNamingService.generateUniqueFileName("invoice", 95, tempDir.toString());
        assertEquals("invoice_95_1.pdf", fileName2);

        // Reset and try again
        fileNamingService.reset();
        String fileName3 = fileNamingService.generateUniqueFileName("invoice", 95, tempDir.toString());
        assertEquals("invoice_95.pdf", fileName3);
    }
}