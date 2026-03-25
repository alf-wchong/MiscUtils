#include <SFML/Graphics.hpp>
#include <windows.h> // Required for Win32 API

// Define the affinity constant if your MinGW headers are older
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Secure Glitcher", sf::Style::Fullscreen);
    
    // 1. Get the native Win32 Window Handle (HWND)
    HWND hwnd = window.getSystemHandle();

    // 2. Apply the Display Affinity
    // This makes the window appear black/invisible to screen capture software
    if (SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE)) {
        // Success: Print Screen/OBS will now see a black hole here
    }

    window.setVerticalSyncEnabled(false);

    float phase = 0.0f;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || 
               (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                window.close();
        }

        window.clear(sf::Color::Black);

        // 3. The Physical "Camera Glitch" Pattern
        // Adding a 'phase' shift makes the lines drift, increasing the Moiré chaos
        phase += 0.5f; 
        for (float x = 0; x < 1920; x += 2.0f) {
            float xPos = x + (int)phase % 2; 
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(xPos, 0), sf::Color::White),
                sf::Vertex(sf::Vector2f(xPos, 1080), sf::Color::White)
            };
            window.draw(line, 2, sf::Lines);
        }

        window.display();
    }
    return 0;
}
