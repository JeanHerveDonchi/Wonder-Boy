
#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1024, 768), "SFML works!");
    sf::CircleShape shape(20.f);
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(20.f, 20.f);
    shape.setPosition(1024 / 2.f, 768 / 2.f);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}