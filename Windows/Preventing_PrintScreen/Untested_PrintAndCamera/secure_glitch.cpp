#include <SFML/Graphics.hpp>
#include <windows.h>
#include <iostream>

#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

int main() {
    // 1. Setup Window (Fullscreen for maximum protection area)
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Secure Display", sf::Style::Fullscreen);
    window.setVerticalSyncEnabled(false);

    // 2. Apply Win32 Display Affinity
    HWND hwnd = window.getSystemHandle();
    if (!SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE)) {
        std::cerr << "Failed to set display affinity." << std::endl;
    }

    // 3. Load Font for the "Human Only" Message
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) { 
        // If font fails, the app will still run with the grid
        std::cerr << "Font not found! Place arial.ttf in the app folder." << std::endl;
    }

    sf::Text message;
    message.setFont(font);
    message.setString("Only the Human eyes this,\nnot the camera nor the print-screen or screen-share.");
    message.setCharacterSize(50); 
    message.setFillColor(sf::Color::White);
    message.setStyle(sf::Text::Bold);

    // Center the text
    sf::FloatRect textRect = message.getLocalBounds();
    message.setOrigin(textRect.left + textRect.width / 2.0f,
                     textRect.top + textRect.height / 2.0f);
    message.setPosition(sf::Vector2f(1920 / 2.0f, 1080 / 2.0f));

    float phase = 0.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || 
               (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                window.close();
        }

        window.clear(sf::Color::Black);

        // 4. Draw the Anti-Camera Grid (Background Layer)
        phase += 0.5f; 
        for (float x = 0; x < 1920; x += 2.0f) {
            float xPos = x + (int)phase % 2; 
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(xPos, 0), sf::Color(100, 100, 100)), // Dimmer white for better text contrast
                sf::Vertex(sf::Vector2f(xPos, 1080), sf::Color(100, 100, 100))
            };
            window.draw(line, 2, sf::Lines);
        }

        // 5. Draw the Human-Readable Text (Foreground Layer)
        window.draw(message);

        window.display();
    }

    return 0;
}
