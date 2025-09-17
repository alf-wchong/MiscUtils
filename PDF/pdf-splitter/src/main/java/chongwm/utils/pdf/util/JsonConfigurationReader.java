package chongwm.utils.pdf.util;

import com.fasterxml.jackson.databind.ObjectMapper;
import chongwm.utils.pdf.exception.InvalidConfigurationException;
import chongwm.utils.pdf.model.SplitConfiguration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;

/**
 * Utility class for reading JSON configuration files.
 */
public class JsonConfigurationReader {
    
    private static final Logger logger = LoggerFactory.getLogger(JsonConfigurationReader.class);
    private final ObjectMapper objectMapper;

    public JsonConfigurationReader() {
        this.objectMapper = new ObjectMapper();
    }

    /**
     * Reads and parses a JSON configuration file.
     * 
     * @param configFilePath path to the JSON configuration file
     * @return parsed SplitConfiguration
     * @throws InvalidConfigurationException if the configuration is invalid
     */
    public SplitConfiguration readConfiguration(String configFilePath) throws InvalidConfigurationException {
        logger.info("Reading configuration from: {}", configFilePath);
        
        File configFile = new File(configFilePath);
        if (!configFile.exists()) {
            throw new InvalidConfigurationException("Configuration file does not exist: " + configFilePath);
        }
        
        if (!configFile.canRead()) {
            throw new InvalidConfigurationException("Cannot read configuration file: " + configFilePath);
        }
        
        try {
            SplitConfiguration config = objectMapper.readValue(configFile, SplitConfiguration.class);
            config.validate();
            
            logger.info("Successfully read configuration with {} document sections", 
                       config.getDocuments().size());
            return config;
            
        } catch (IOException e) {
            throw new InvalidConfigurationException("Failed to parse JSON configuration: " + e.getMessage(), e);
        } catch (IllegalArgumentException e) {
            throw new InvalidConfigurationException("Invalid configuration: " + e.getMessage(), e);
        }
    }
}
