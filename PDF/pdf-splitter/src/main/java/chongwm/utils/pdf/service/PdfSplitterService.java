package chongwm.utils.pdf.service;

import chongwm.utils.pdf.exception.PdfSplitterException;
import chongwm.utils.pdf.model.DocumentSection;
import chongwm.utils.pdf.model.SplitConfiguration;
import org.apache.pdfbox.multipdf.Splitter;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Service for splitting PDF documents based on configuration.
 */
public class PdfSplitterService {
    
    private static final Logger logger = LoggerFactory.getLogger(PdfSplitterService.class);
    
    private final FileNamingService fileNamingService;

    public PdfSplitterService(FileNamingService fileNamingService) {
        this.fileNamingService = fileNamingService;
    }

    public PdfSplitterService() {
        this(new FileNamingService());
    }

    /**
     * Splits a PDF file according to the provided configuration.
     * 
     * @param inputPdfPath path to the input PDF file
     * @param configurationPath path to the JSON configuration file
     * @param outputDirectory directory where split PDFs will be saved
     * @return list of generated output file paths
     * @throws PdfSplitterException if splitting fails
     */
    public List<String> splitPdf(String inputPdfPath, String configurationPath, String outputDirectory) 
            throws PdfSplitterException {
        
        logger.info("Starting PDF split operation");
        logger.info("Input PDF: {}", inputPdfPath);
        logger.info("Configuration: {}", configurationPath);
        logger.info("Output directory: {}", outputDirectory);
        
        // Validate input parameters
        validateInputs(inputPdfPath, configurationPath, outputDirectory);
        
        // Read configuration
        SplitConfiguration config = readConfiguration(configurationPath);
        
        // Split PDF
        return splitPdfWithConfiguration(inputPdfPath, config, outputDirectory);
    }

    /**
     * Splits a PDF file with a provided configuration object.
     * 
     * @param inputPdfPath path to the input PDF file
     * @param config the split configuration
     * @param outputDirectory directory where split PDFs will be saved
     * @return list of generated output file paths
     * @throws PdfSplitterException if splitting fails
     */
    public List<String> splitPdfWithConfiguration(String inputPdfPath, SplitConfiguration config, 
                                                 String outputDirectory) throws PdfSplitterException {
        
        List<String> outputFiles = new ArrayList<>();
        
        try (PDDocument document = PDDocument.load(new File(inputPdfPath))) {
            int totalPages = document.getNumberOfPages();
            logger.info("Input PDF has {} pages", totalPages);
            
            // Validate page ranges against actual document
            validatePageRanges(config, totalPages);
            
            // Create output directory if it doesn't exist
            File outputDir = new File(outputDirectory);
            if (!outputDir.exists() && !outputDir.mkdirs()) {
                throw new PdfSplitterException("Failed to create output directory: " + outputDirectory);
            }
            
            // Process each document section
            for (DocumentSection section : config.getDocuments()) {
                String outputFile = extractDocumentSection(document, section, outputDirectory);
                outputFiles.add(outputFile);
            }
            
            logger.info("Successfully split PDF into {} files", outputFiles.size());
            
        } catch (IOException e) {
            throw new PdfSplitterException("Failed to process PDF file: " + e.getMessage(), e);
        }
        
        return outputFiles;
    }

    /**
     * Extracts a specific document section and saves it as a separate PDF.
     * 
     * @param sourceDocument the source PDF document
     * @param section the document section to extract
     * @param outputDirectory the output directory
     * @return path to the generated file
     * @throws PdfSplitterException if extraction fails
     */
    private String extractDocumentSection(PDDocument sourceDocument, DocumentSection section, 
                                        String outputDirectory) throws PdfSplitterException {
        
        logger.debug("Extracting section: {}", section);
        
        try {
            // Create a new document for this section
            PDDocument sectionDocument = new PDDocument();
            
            // Copy pages from source to section document (PDFBox uses 0-based indexing)
            int startPageIndex = section.getStartPage() - 1;
            int endPageIndex = section.getEndPage() - 1;
            
            for (int i = startPageIndex; i <= endPageIndex; i++) {
                sectionDocument.addPage(sourceDocument.getPage(i));
            }
            
            // Generate unique filename
            String fileName = fileNamingService.generateUniqueFileName(
                section.getCategory(), section.getConfidence(), outputDirectory);
            
            String outputPath = outputDirectory + File.separator + fileName;
            
            // Save the section document
            sectionDocument.save(outputPath);
            sectionDocument.close();
            
            logger.info("Created file: {} (pages {}-{})", fileName, 
                       section.getStartPage(), section.getEndPage());
            
            return outputPath;
            
        } catch (IOException e) {
            throw new PdfSplitterException(
                String.format("Failed to extract section %s: %s", section.toString(), e.getMessage()), e);
        }
    }

    /**
     * Validates input parameters.
     * 
     * @param inputPdfPath path to input PDF
     * @param configurationPath path to configuration file
     * @param outputDirectory output directory path
     * @throws PdfSplitterException if validation fails
     */
    private void validateInputs(String inputPdfPath, String configurationPath, String outputDirectory) 
            throws PdfSplitterException {
        
        // Check input PDF
        File inputFile = new File(inputPdfPath);
        if (!inputFile.exists()) {
            throw new PdfSplitterException("Input PDF file does not exist: " + inputPdfPath);
        }
        if (!inputFile.canRead()) {
            throw new PdfSplitterException("Cannot read input PDF file: " + inputPdfPath);
        }
        
        // Check configuration file
        File configFile = new File(configurationPath);
        if (!configFile.exists()) {
            throw new PdfSplitterException("Configuration file does not exist: " + configurationPath);
        }
        if (!configFile.canRead()) {
            throw new PdfSplitterException("Cannot read configuration file: " + configurationPath);
        }
        
        // Check output directory
        File outputDir = new File(outputDirectory);
        if (outputDir.exists() && !outputDir.isDirectory()) {
            throw new PdfSplitterException("Output path exists but is not a directory: " + outputDirectory);
        }
        
        File parentDir = outputDir.getParentFile();
        if (parentDir != null && !parentDir.canWrite()) {
            throw new PdfSplitterException("Cannot write to output directory parent: " + parentDir.getAbsolutePath());
        }
    }

    /**
     * Reads and validates the configuration file.
     * 
     * @param configurationPath path to configuration file
     * @return parsed configuration
     * @throws PdfSplitterException if reading fails
     */
    private SplitConfiguration readConfiguration(String configurationPath) throws PdfSplitterException {
        try {
            return new chongwm.utils.pdf.util.JsonConfigurationReader().readConfiguration(configurationPath);
        } catch (chongwm.utils.pdf.exception.InvalidConfigurationException e) {
            throw new PdfSplitterException("Configuration error: " + e.getMessage(), e);
        }
    }

    /**
     * Validates that all page ranges are within the document bounds.
     * 
     * @param config the configuration to validate
     * @param totalPages total pages in the document
     * @throws PdfSplitterException if validation fails
     */
    private void validatePageRanges(SplitConfiguration config, int totalPages) throws PdfSplitterException {
        for (DocumentSection section : config.getDocuments()) {
            if (section.getStartPage() > totalPages) {
                throw new PdfSplitterException(
                    String.format("Start page %d exceeds document page count %d for section %s",
                                section.getStartPage(), totalPages, section.getCategory()));
            }
            if (section.getEndPage() > totalPages) {
                throw new PdfSplitterException(
                    String.format("End page %d exceeds document page count %d for section %s",
                                section.getEndPage(), totalPages, section.getCategory()));
            }
        }
    }
}
