package chongwm.utils.pdf.model;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.List;

/**
 * Represents the complete configuration for PDF splitting.
 */
public class SplitConfiguration {
    
    @JsonProperty("documents")
    private List<DocumentSection> documents;

    public SplitConfiguration() {
        // Default constructor for Jackson
    }

    public SplitConfiguration(List<DocumentSection> documents) {
        this.documents = documents;
    }

    public List<DocumentSection> getDocuments() {
        return documents;
    }

    public void setDocuments(List<DocumentSection> documents) {
        this.documents = documents;
    }

    /**
     * Validates the entire configuration.
     * 
     * @throws IllegalArgumentException if the configuration is invalid
     */
    public void validate() {
        if (documents == null || documents.isEmpty()) {
            throw new IllegalArgumentException("Documents list cannot be null or empty");
        }
        
        // Validate each document section
        for (DocumentSection doc : documents) {
            doc.validate();
        }
        
        // Check for overlapping page ranges
        for (int i = 0; i < documents.size(); i++) {
            for (int j = i + 1; j < documents.size(); j++) {
                DocumentSection doc1 = documents.get(i);
                DocumentSection doc2 = documents.get(j);
                
                if (rangesOverlap(doc1.getStartPage(), doc1.getEndPage(), 
                                doc2.getStartPage(), doc2.getEndPage())) {
                    throw new IllegalArgumentException(
                        String.format("Page ranges overlap between documents: %s and %s", 
                                    doc1.toString(), doc2.toString()));
                }
            }
        }
    }
    
    private boolean rangesOverlap(int start1, int end1, int start2, int end2) {
        return start1 <= end2 && start2 <= end1;
    }

    @Override
    public String toString() {
        return String.format("SplitConfiguration{documents=%s}", documents);
    }
}