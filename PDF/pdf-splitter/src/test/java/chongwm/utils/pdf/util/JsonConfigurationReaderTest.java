package chongwm.utils.pdf.util;

import chongwm.utils.pdf.exception.InvalidConfigurationException;
import chongwm.utils.pdf.model.SplitConfiguration;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.*;

class JsonConfigurationReaderTest {

    private JsonConfigurationReader reader;

    @TempDir
    Path tempDir;

    @BeforeEach
    void setUp() {
        reader = new JsonConfigurationReader();
    }

    @Test
    void testReadValidConfiguration() throws IOException, InvalidConfigurationException {
        File configFile = createConfigFile(
            "{\n" +
            "  \"documents\": [\n" +
            "    {\n" +
            "      \"category\": \"invoice\",\n" +
            "      \"start_page\": 1,\n" +
            "      \"end_page\": 2,\n" +
            "      \"confidence\": 95\n" +
            "    },\n" +
            "    {\n" +
            "      \"category\": \"receipt\",\n" +
            "      \"start_page\": 3,\n" +
            "      \"end_page\": 4,\n" +
            "      \"confidence\": 88\n" +
            "    }\n" +
            "  ]\n" +
            "}"
        );

        SplitConfiguration config = reader.readConfiguration(configFile.getAbsolutePath());
        
        assertNotNull(config);
        assertEquals(2, config.getDocuments().size());
        assertEquals("invoice", config.getDocuments().get(0).getCategory());
        assertEquals(95, config.getDocuments().get(0).getConfidence());
    }

    @Test
    void testReadConfigurationWithInvalidJson() throws IOException {
        File configFile = createConfigFile("{ invalid json }");

        assertThrows(InvalidConfigurationException.class, () -> {
            reader.readConfiguration(configFile.getAbsolutePath());
        });
    }

    @Test
    void testReadConfigurationWithMissingFields() throws IOException {
        File configFile = createConfigFile(
            "{\n" +
            "  \"documents\": [\n" +
            "    {\n" +
            "      \"category\": \"invoice\",\n" +
            "      \"start_page\": 1\n" +
            "    }\n" +
            "  ]\n" +
            "}"
        );

        assertThrows(InvalidConfigurationException.class, () -> {
            reader.readConfiguration(configFile.getAbsolutePath());
        });
    }

    @Test
    void testReadConfigurationWithInvalidPageRange() throws IOException {
        File configFile = createConfigFile(
            "{\n" +
            "  \"documents\": [\n" +
            "    {\n" +
            "      \"category\": \"invoice\",\n" +
            "      \"start_page\": 5,\n" +
            "      \"end_page\": 2,\n" +
            "      \"confidence\": 95\n" +
            "    }\n" +
            "  ]\n" +
            "}"
        );

        assertThrows(InvalidConfigurationException.class, () -> {
            reader.readConfiguration(configFile.getAbsolutePath());
        });
    }

    @Test
    void testReadConfigurationWithOverlappingRanges() throws IOException {
        File configFile = createConfigFile(
            "{\n" +
            "  \"documents\": [\n" +
            "    {\n" +
            "      \"category\": \"doc1\",\n" +
            "      \"start_page\": 1,\n" +
            "      \"end_page\": 3,\n" +
            "      \"confidence\": 95\n" +
            "    },\n" +
            "    {\n" +
            "      \"category\": \"doc2\",\n" +
            "      \"start_page\": 2,\n" +
            "      \"end_page\": 4,\n" +
            "      \"confidence\": 88\n" +
            "    }\n" +
            "  ]\n" +
            "}"
        );

        assertThrows(InvalidConfigurationException.class, () -> {
            reader.readConfiguration(configFile.getAbsolutePath());
        });
    }

    @Test
    void testReadNonExistentConfiguration() {
        assertThrows(InvalidConfigurationException.class, () -> {
            reader.readConfiguration("nonexistent.json");
        });
    }

    private File createConfigFile(String content) throws IOException {
        File configFile = tempDir.resolve("test-config.json").toFile();
        try (FileWriter writer = new FileWriter(configFile)) {
            writer.write(content);
        }
        return configFile;
    }
}
