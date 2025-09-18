package chongwm.utils.pdf.counter;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Represents a document entry in the JSON schema
 */
public class Document {
    @JsonProperty("category")
    private String category;
    
    @JsonProperty("start_page")
    private int startPage;
    
    @JsonProperty("end_page")
    private int endPage;
    
    @JsonProperty("confidence")
    private int confidence;
    
    @JsonProperty("incomplete")
    private Boolean incomplete;
    
    @JsonProperty("template")
    private Boolean template;

    // Default constructor for Jackson
    public Document() {}

    // Getters and setters
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

    public Boolean getIncomplete() {
        return incomplete;
    }

    public void setIncomplete(Boolean incomplete) {
        this.incomplete = incomplete;
    }

    public Boolean getTemplate() {
        return template;
    }

    public void setTemplate(Boolean template) {
        this.template = template;
    }

    /**
     * Calculate the number of pages in this document
     */
    public int getPageCount() {
        return endPage - startPage + 1;
    }

    @Override
    public String toString() {
        return "Document{" +
                "category='" + category + '\'' +
                ", startPage=" + startPage +
                ", endPage=" + endPage +
                ", confidence=" + confidence +
                ", incomplete=" + incomplete +
                ", template=" + template +
                '}';
    }
}