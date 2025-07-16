#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

int main() {
    sf::RenderWindow window(sf::VideoMode(600, 400), "SFML Chat GUI");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        return -1; // Make sure this path is valid or adjust it
    }

    std::vector<std::string> messages;
    std::string currentInput = "";
    float scrollOffset = 0.0f;

    sf::Text inputText;
    inputText.setFont(font);
    inputText.setCharacterSize(18);
    inputText.setFillColor(sf::Color::White);
    inputText.setPosition(10, 360);

    sf::RectangleShape inputBox(sf::Vector2f(580, 30));
    inputBox.setPosition(10, 355);
    inputBox.setFillColor(sf::Color(50, 50, 50));

	int i = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // Close window
            if (event.type == sf::Event::Closed)
                window.close();

            // Mouse wheel scrolling
            if (event.type == sf::Event::MouseWheelScrolled) {
                scrollOffset += event.mouseWheelScroll.delta * 20;
                if (scrollOffset < 0) scrollOffset = 0;
            }

            // Handle typing input
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b' && !currentInput.empty()) {
                    currentInput.pop_back();
                } else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                    if (!currentInput.empty()) {
                        messages.push_back(currentInput);
                        currentInput.clear();
                    }
                } else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                    currentInput += static_cast<char>(event.text.unicode);
                }
            }
        }
		i++;

        window.clear(sf::Color(30, 30, 30));

        // Draw messages
        int visibleLines = 15;
        float startY = 10;
        float y = startY;
        int startIndex = std::max(0, static_cast<int>(messages.size()) - visibleLines);

        for (int i = 0; i < messages.size(); ++i) {
            sf::Text message;
            message.setFont(font);
            message.setCharacterSize(16);
            message.setFillColor(sf::Color::Cyan);
            message.setString(messages[i]);
            message.setPosition(10, y - scrollOffset);
            y += 20;
            if (message.getPosition().y > 0 && message.getPosition().y < 360)
                window.draw(message);
        }

        // Draw input box
        window.draw(inputBox);
        if (i == 5000)
        {
        	i = 0;
        	inputText.setString(currentInput);
        }
        else 
			inputText.setString(currentInput + "|");
		
        window.draw(inputText);

        window.display();
    }

    return 0;
}
