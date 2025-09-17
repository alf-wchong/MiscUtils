package chongwm.utils.pdf.model;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Represents a document section configuration for PDF splitting.
 */
public class DocumentSection {
    
    @JsonProperty("category")
    private String category;
    
    @JsonProperty("start_page")
    private int startPage;
    
    @JsonProperty("end_page")
    private int endPage;
    
    @JsonProperty("confidence")
    private int confidence;

    public DocumentSection() {
        // Default constructor for Jackson
    }

    public DocumentSection(String category, int startPage, int endPage, int confidence) {
        this.category = category;
        this.startPage = startPage;
        this.endPage = endPage;
        this.confidence = confidence;
    }

    public String getCategory() {
        return category;
    }

    public void setCategory(String category) {
        this.category = category;
    }

    public int getStartPage() {
        return startPage;
    }

    public void setStartPage(int startPage) {
        this.startPage = startPage;
    }

    public int getEndPage() {
        return endPage;
    }

    public void setEndPage(int endPage) {
        this.endPage = endPage;
    }

    public int getConfidence() {
        return confidence;
    }

    public void setConfidence(int confidence) {
        this.confidence = confidence;
    }

    /**
     * Validates the document section configuration.
     * 
     * @throws IllegalArgumentException if the configuration is invalid
     */
    public void validate() {
        if (category == null || category.trim().isEmpty()) {
            throw new IllegalArgumentException("Category cannot be null or empty");
        }
        if (startPage < 1) {
            throw new IllegalArgumentException("Start page must be greater than 0");
        }
        if (endPage < 1) {
            throw new IllegalArgumentException("End page must be greater than 0");
        }
        if (startPage > endPage) {
            throw new IllegalArgumentException("Start page cannot be greater than end page");
        }
        if (confidence < 0 || confidence > 100) {
            throw new IllegalArgumentException("Confidence must be between 0 and 100");
        }
    }

    @Override
    public String toString() {
        return String.format("DocumentSection{category='%s', startPage=%d, endPage=%d, confidence=%d}",
                category, startPage, endPage, confidence);
    }
}
