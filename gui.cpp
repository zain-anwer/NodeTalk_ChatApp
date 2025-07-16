#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unistd.h>

using namespace std;

int main()
{
	sf::RenderWindow window (sf::VideoMode(600,400),"Parlons");
	window.setFramerateLimit(60);

	sf::Font font;
	if (font.loadFromFile("Lato-Regular.ttf") == 0)
		return -1;

	sf::Text input_text;

	input_text.setFont(font);
	input_text.setCharacterSize(18);
	input_text.setFillColor(sf::Color::White);
	input_text.setPosition(10,360);

	sf::RectangleShape message_box (sf::Vector2f(580,30));
	message_box.setPosition(10,355);
	message_box.setFillColor(sf::Color(50,50,50));

	int i = 0;
	vector<string> messages;
	string current_input = "";
			
	while(window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::TextEntered)
			{
				if (event.text.unicode == '\b' && !current_input.empty())
					current_input.pop_back();
				if (event.text.unicode == '\r' || event.text.unicode == '\n')
				{
					if (!current_input.empty())
						messages.push_back(current_input);
					current_input.clear();
				}
				else if (event.text.unicode < 128 && event.text.unicode != '\b')
					 current_input += static_cast<char>(event.text.unicode);
			}
		}
		window.clear(sf::Color(30,30,30));	
		window.draw(message_box);
		window.draw(input_text);

		int y = 0;

		for (int i = 0 ; i < static_cast<int>(messages.size()) ; i++)
		{
			sf::Text message; 
			message.setFont(font);
			message.setString(messages[i]);
			message.setCharacterSize(18);
			message.setFillColor(sf::Color(255, 192, 203));
			message.setPosition(10,y);
			y += 25;
			if (y > 360)
				break;
			window.draw(message);
		}

		i++;
		 
        if (i < 30)
        	input_text.setString(current_input + "|");
        else if (i < 60)
			input_text.setString(current_input);
		else
			i = 0;
		window.display();
	}
	
	return 0;
}
