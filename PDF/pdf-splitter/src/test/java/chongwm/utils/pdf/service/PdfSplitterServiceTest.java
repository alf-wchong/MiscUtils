package chongwm.utils.pdf.service;

import chongwm.utils.pdf.exception.PdfSplitterException;
import chongwm.utils.pdf.model.DocumentSection;
import chongwm.utils.pdf.model.SplitConfiguration;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.apache.pdfbox.pdmodel.PDPage;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

class PdfSplitterServiceTest {

    private PdfSplitterService pdfSplitterService;
    private File testPdf;

    @TempDir
    Path tempDir;

    @BeforeEach
    void setUp() throws IOException {
        pdfSplitterService = new PdfSplitterService();
        testPdf = createTestPdf();
    }

    @Test
    void testSplitPdfWithValidConfiguration() throws PdfSplitterException {
        // Create test configuration
        List<DocumentSection> sections = Arrays.asList(
            new DocumentSection("invoice", 1, 2, 95),
            new DocumentSection("receipt", 3, 4, 88),
            new DocumentSection("contract", 5, 5, 92)
        );
        SplitConfiguration config = new SplitConfiguration(sections);

        // Split PDF
        List<String> outputFiles = pdfSplitterService.splitPdfWithConfiguration(
            testPdf.getAbsolutePath(), config, tempDir.toString());

        // Verify results
        assertEquals(3, outputFiles.size());
        
        for (String outputFile : outputFiles) {
            File file = new File(outputFile);
            assertTrue(file.exists());
            assertTrue(file.length() > 0);
        }
    }

    @Test
    void testSplitPdfWithDuplicateCategories() throws PdfSplitterException {
        // Create configuration with duplicate categories
        List<DocumentSection> sections = Arrays.asList(
            new DocumentSection("invoice", 1, 1, 95),
            new DocumentSection("invoice", 2, 2, 88),
            new DocumentSection("invoice", 3, 3, 92)
        );
        SplitConfiguration config = new SplitConfiguration(sections);

        // Split PDF
        List<String> outputFiles = pdfSplitterService.splitPdfWithConfiguration(
            testPdf.getAbsolutePath(), config, tempDir.toString());

        // Verify unique filenames were generated
        assertEquals(3, outputFiles.size());
        
        // Check that all files exist and have unique names
        for (String outputFile : outputFiles) {
            File file = new File(outputFile);
            assertTrue(file.exists());
        }
        
        // Verify filenames are unique
        long uniqueCount = outputFiles.stream()
            .map(path -> new File(path).getName())
            .distinct()
            .count();
        assertEquals(3, uniqueCount);
    }

    @Test
    void testSplitPdfWithInvalidPageRange() {
        // Create configuration with page range exceeding document
        List<DocumentSection> sections = Arrays.asList(
            new DocumentSection("invoice", 1, 10, 95) // PDF only has 5 pages
        );
        SplitConfiguration config = new SplitConfiguration(sections);

        // Should throw exception
        assertThrows(PdfSplitterException.class, () -> {
            pdfSplitterService.splitPdfWithConfiguration(
                testPdf.getAbsolutePath(), config, tempDir.toString());
        });
    }

    @Test
    void testSplitPdfWithNonExistentInputFile() {
        List<DocumentSection> sections = Arrays.asList(
            new DocumentSection("test", 1, 1, 95)
        );
        SplitConfiguration config = new SplitConfiguration(sections);

        assertThrows(PdfSplitterException.class, () -> {
            pdfSplitterService.splitPdfWithConfiguration(
                "nonexistent.pdf", config, tempDir.toString());
        });
    }

    private File createTestPdf() throws IOException {
        File pdfFile = tempDir.resolve("test.pdf").toFile();
        
        PDDocument document = new PDDocument();
        // Add 5 pages to the test PDF
        for (int i = 0; i < 5; i++) {
            document.addPage(new PDPage());
        }
        
        document.save(pdfFile);
        document.close();
        
        return pdfFile;
    }
}
