package chongwm.utils.pdf;

import chongwm.utils.pdf.exception.PdfSplitterException;
import chongwm.utils.pdf.service.PdfSplitterService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

/**
 * Main application class for PDF splitting.
 */
public class PdfSplitterApplication {
    
    private static final Logger logger = LoggerFactory.getLogger(PdfSplitterApplication.class);

    public static void main(String[] args) {
        if (args.length != 3) {
            System.err.println("Usage: java -jar pdf-splitter.jar <input-pdf> <config-json> <output-directory>");
            System.err.println("Example: java -jar pdf-splitter.jar input.pdf config.json ./output/");
            System.exit(1);
        }

        String inputPdfPath = args[0];
        String configurationPath = args[1];
        String outputDirectory = args[2];

        try {
            logger.info("Starting PDF Splitter Application");
            
            PdfSplitterService service = new PdfSplitterService();
            List<String> outputFiles = service.splitPdf(inputPdfPath, configurationPath, outputDirectory);
            
            System.out.println("PDF splitting completed successfully!");
            System.out.println("Generated files:");
            for (String file : outputFiles) {
                System.out.println("  " + file);
            }
            
            logger.info("Application completed successfully");
            
        } catch (PdfSplitterException e) {
            logger.error("PDF splitting failed: {}", e.getMessage(), e);
            System.err.println("Error: " + e.getMessage());
            System.exit(1);
        } catch (Exception e) {
            logger.error("Unexpected error: {}", e.getMessage(), e);
            System.err.println("Unexpected error: " + e.getMessage());
            System.exit(1);
        }
    }
}