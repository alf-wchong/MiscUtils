package chongwm.utils.pdf.counter;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.apache.pdfbox.Loader;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;

/**
 * Main application class for PDF page counting and comparison
 */
public class PdfPageCounter {
    private static final Logger logger = LoggerFactory.getLogger(PdfPageCounter.class);
    private static final String USAGE = "Usage: java -jar counter.jar <pdf_file> <json_file> [-o <output_mode>]\n" +
                                       "  output_mode: console (default) or f (file)";

    public static void main(String[] args) {
        try {
            ArgumentParser parser = new ArgumentParser(args);
            String pdfFilePath = parser.getPdfFilePath();
            String jsonFilePath = parser.getJsonFilePath();
            String outputMode = parser.getOutputMode();

            PdfPageCounter counter = new PdfPageCounter();
            String result = counter.comparePageCounts(pdfFilePath, jsonFilePath);
            
            counter.outputResult(result, pdfFilePath, outputMode);
            
        } catch (PdfCounterException e) {
            logger.error("Error: {}", e.getMessage());
            System.exit(e.getErrorCode());
        } catch (Exception e) {
            logger.error("Unexpected error: {}", e.getMessage(), e);
            System.exit(-99);
        }
    }

    /**
     * Compare PDF page count with JSON document definitions
     */
    public String comparePageCounts(String pdfFilePath, String jsonFilePath) throws PdfCounterException {
        int pdfPageCount = countPdfPages(pdfFilePath);
        int jsonPageCount = countJsonPages(jsonFilePath);
        
        int difference = pdfPageCount - jsonPageCount;
        
        if (difference > 0) {
            return "+" + difference;
        } else if (difference < 0) {
            return String.valueOf(difference);
        } else {
            return "0";
        }
    }

    /**
     * Count pages in a PDF file
     */
    private int countPdfPages(String pdfFilePath) throws PdfCounterException {
        File pdfFile = new File(pdfFilePath);
        
        if (!pdfFile.exists()) {
            throw new PdfCounterException("PDF file not found: " + pdfFilePath, 
                                        PdfCounterException.PDF_FILE_NOT_FOUND);
        }
        
        if (!pdfFile.canRead()) {
            throw new PdfCounterException("Cannot read PDF file: " + pdfFilePath, 
                                        PdfCounterException.INVALID_PDF);
        }

        try (PDDocument document = Loader.loadPDF(pdfFile)) {
            int pageCount = document.getNumberOfPages();
            logger.info("PDF page count: {}", pageCount);
            return pageCount;
        } catch (IOException e) {
            throw new PdfCounterException("Error reading PDF file: " + e.getMessage(), 
                                        PdfCounterException.INVALID_PDF, e);
        }
    }

    /**
     * Count pages from JSON document definitions
     */
    private int countJsonPages(String jsonFilePath) throws PdfCounterException {
        File jsonFile = new File(jsonFilePath);
        
        if (!jsonFile.exists()) {
            throw new PdfCounterException("JSON file not found: " + jsonFilePath, 
                                        PdfCounterException.JSON_FILE_NOT_FOUND);
        }

        try {
            ObjectMapper mapper = new ObjectMapper();
            DocumentList documentList = mapper.readValue(jsonFile, DocumentList.class);
            
            if (documentList.getDocuments() == null || documentList.getDocuments().isEmpty()) {
                logger.info("No documents found in JSON, page count: 0");
                return 0;
            }

            validatePageRanges(documentList.getDocuments());
            
            int totalPages = documentList.getDocuments().stream()
                    .mapToInt(Document::getPageCount)
                    .sum();
            
            logger.info("JSON page count: {}", totalPages);
            return totalPages;
            
        } catch (IOException e) {
            throw new PdfCounterException("Error reading JSON file: " + e.getMessage(), 
                                        PdfCounterException.INVALID_JSON, e);
        }
    }

    /**
     * Validate page ranges for overlaps and invalid ranges
     */
    private void validatePageRanges(List<Document> documents) throws PdfCounterException {
        // Check for invalid page ranges
        for (Document doc : documents) {
            if (doc.getStartPage() > doc.getEndPage()) {
                throw new PdfCounterException("Invalid page range: start_page (" + doc.getStartPage() + 
                                            ") is greater than end_page (" + doc.getEndPage() + ")", 
                                            PdfCounterException.INVALID_PAGE_RANGE);
            }
            if (doc.getStartPage() < 1) {
                throw new PdfCounterException("Invalid start_page: " + doc.getStartPage() + " (must be >= 1)", 
                                            PdfCounterException.INVALID_PAGE_RANGE);
            }
        }

        // Check for overlapping page ranges
        for (int i = 0; i < documents.size(); i++) {
            for (int j = i + 1; j < documents.size(); j++) {
                Document doc1 = documents.get(i);
                Document doc2 = documents.get(j);
                
                if (isOverlapping(doc1, doc2)) {
                    throw new PdfCounterException("Overlapping page ranges detected between documents: " +
                                                "Document 1 (pages " + doc1.getStartPage() + "-" + doc1.getEndPage() + ") " +
                                                "and Document 2 (pages " + doc2.getStartPage() + "-" + doc2.getEndPage() + ")",
                                                PdfCounterException.OVERLAPPING_PAGE_RANGES);
                }
            }
        }
    }

    /**
     * Check if two documents have overlapping page ranges
     */
    private boolean isOverlapping(Document doc1, Document doc2) {
        return doc1.getStartPage() <= doc2.getEndPage() && doc2.getStartPage() <= doc1.getEndPage();
    }

    /**
     * Output result to console or file
     */
    private void outputResult(String result, String pdfFilePath, String outputMode) throws PdfCounterException {
        if ("f".equals(outputMode)) {
            outputToFile(result, pdfFilePath);
        } else {
            System.out.println(result);
        }
    }

    /**
     * Output result to file
     */
    private void outputToFile(String result, String pdfFilePath) throws PdfCounterException {
        try {
            Path outputPath = Paths.get(pdfFilePath + ".result");
            Files.write(outputPath, result.getBytes());
            logger.info("Result written to file: {}", outputPath.toAbsolutePath());
        } catch (IOException e) {
            throw new PdfCounterException("Error writing output file: " + e.getMessage(), 
                                        PdfCounterException.OUTPUT_FILE_ERROR, e);
        }
    }

    /**
     * Inner class to handle command line argument parsing
     */
    private static class ArgumentParser {
        private String pdfFilePath;
        private String jsonFilePath;
        private String outputMode = "console";

        public ArgumentParser(String[] args) throws PdfCounterException {
            if (args.length < 2) {
                throw new PdfCounterException("Insufficient arguments provided.\n" + USAGE, 
                                            PdfCounterException.INVALID_ARGUMENTS);
            }

            pdfFilePath = args[0];
            jsonFilePath = args[1];

            // Parse optional output mode
            for (int i = 2; i < args.length; i++) {
                if ("-o".equals(args[i])) {
                    if (i + 1 < args.length) {
                        outputMode = args[i + 1];
                        if (!"console".equals(outputMode) && !"f".equals(outputMode)) {
                            throw new PdfCounterException("Invalid output mode: " + outputMode + 
                                                        " (must be 'console' or 'f')\n" + USAGE, 
                                                        PdfCounterException.INVALID_ARGUMENTS);
                        }
                        i++; // Skip the next argument as it's the value for -o
                    } else {
                        throw new PdfCounterException("Missing value for -o parameter\n" + USAGE, 
                                                    PdfCounterException.INVALID_ARGUMENTS);
                    }
                }
            }
        }

        public String getPdfFilePath() {
            return pdfFilePath;
        }

        public String getJsonFilePath() {
            return jsonFilePath;
        }

        public String getOutputMode() {
            return outputMode;
        }
    }
}