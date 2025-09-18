package chongwm.utils.pdf.counter;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.List;

/**
 * Root object for the JSON document schema
 */
public class DocumentList {
    @JsonProperty("documents")
    private List<Document> documents;

    // Default constructor for Jackson
    public DocumentList() {}

    public List<Document> getDocuments() {
        return documents;
    }

    public void setDocuments(List<Document> documents) {
        this.documents = documents;
    }

    @Override
    public String toString() {
        return "DocumentList{" +
                "documents=" + documents +
                '}';
    }
}
