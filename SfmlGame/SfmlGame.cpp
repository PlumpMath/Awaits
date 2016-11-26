
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>

#include <string>
#include <thread>
#include <random>

#include "zassert.h"
#include "sfuture.h"
#include "Guy.h"
#include "ScriptEngine.h"

using namespace std::chrono_literals;

sf::RenderWindow window;
sf::Sound ballSound;
sf::Texture texture;
sf::Sprite sprite;
sf::Text message;

sf::Texture ftexture;
sf::Sprite fsprite;

sf::Vector2i guyFrameSize;

std::vector<Guy> guys { Guy({300, 200}), Guy({600, 300}), Guy({450, 400}), Guy({500, 350}) };

void updateSprite(const Guy& guy, sf::Sprite& sprite)
{
    sprite.setPosition(guy.position);

    auto frames = guy.getFrames();
    auto f = guy.animFrame % frames.size();
    int frame = frames[f];
    int frameX = frame % 6;
    int frameY = frame / 6;
    sf::IntRect rect {frameX * guyFrameSize.x, frameY * guyFrameSize.y, guyFrameSize.x, guyFrameSize.y};
    sprite.setTextureRect(rect);
}

void updateMessage(const Guy& guy, sf::Text& text)
{
    text.setPosition(guy.position.x, guy.position.y - 40);
    text.setString(guy.talk);
}

void draw(const Guy& guy)
{
    updateSprite(guy, sprite);
    window.draw(sprite);

    if (!guy.talk.empty())
    {
        updateMessage(guy, message);
        window.draw(message);
    }
}

void loadGuy()
{
    if (!texture.loadFromFile("resources/guybrush.png"))
        zassert(false);

    auto guySize = texture.getSize();
    guyFrameSize.x = guySize.x / 6 ;
    guyFrameSize.y = guySize.y / 3 + 1;

    sprite.setTexture(texture);
    //sprite.setColor(sf::Color(255, 255, 255, 200));
}

void loadForest()
{
    if (!ftexture.loadFromFile("resources/forest.jpg"))
        zassert(false);

    fsprite.setTexture(ftexture);

    auto size = ftexture.getSize();
    fsprite.setScale(800.0f / size.x, 600.0f / size.y);
}

sfuture<void> walkTo(Guy& guy, sf::Vector2f target)
{
    auto diff = target - guy.position;
    auto speed = 3.0f;
    auto frames = static_cast<int>(std::sqrtf(diff.x * diff.x + diff.y * diff.y) / speed);

    guy.facingLeft = (diff.x < 0);
    guy.walking = true;

    for (int i = 0; i < frames; ++i)
    {
        guy.position += diff * (1.0f / frames);
        co_await wait_few_frames(1);
    }

    guy.position = target;
    guy.walking = false;
}

std::random_device rd; // obtain a random number from hardware
std::mt19937 eng(rd()); // seed the generator

float random(float from, float to)
{
    std::uniform_real_distribution<float> distr(from, to);
    return distr(eng);
}

int random(int from, int to)
{
    std::uniform_int_distribution<> distr(from, to);
    return distr(eng);
}

std::vector<std::string> thingsToSay = {
    "C++ is the best.",
    "Herb Sutter rulez!",
    "while(*p++ = *q++);",
    "CRTP is the answer.",
    "I hate C.",
    "Pirates++",
    "Naked pointers' boobs.",
    "I'm in DLL hell.",
};


sfuture<void> thinkDeeply(Guy& guy)
{
    co_await wait_some_time(2s);
    guy.talk.clear();
}

sfuture<void> think(Guy& guy)
{
    guy.talk = "I'm thinking.";
    co_await wait_some_time(1s);
    guy.talk.clear();

    co_await walkTo(guy, guy.position + sf::Vector2f(random(-100.0f, 100.0f), random(-50.0f, 50.0f)));

    guy.talk = thingsToSay[random(0, thingsToSay.size()-1)];
    co_await wait_some_time(1s);
    guy.talk.clear();
}

sfuture<void> ai(Guy& guy)
{
    std::discrete_distribution<> dist({2, 3, 1, 2});
    while (true)
    {
        auto action = dist(eng);
        switch(action)
        {
            case 0:
                co_await think(guy);
                break;
            case 1:
                co_await walkTo(guy, guy.position + sf::Vector2f{random(-100.0f, 100.0f), random(-50.0f, 50.0f)});
                break;
            case 2:
                co_await walkTo(guy, sf::Vector2f{random(100.0f, 700.0f), random(100.0f, 500.0f)});
                break;
            case 3:
                co_await wait_some_time(std::chrono::seconds(random(1, 3)));
                break;
        }
    }
}

sfuture<void> animate(Guy& guy)
{
    while(true)
    {
        guy.advanceFrame();
        co_await wait_few_frames(4);
    }
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // Define some constants
    const float pi = 3.14159f;
    const int gameWidth = 800;
    const int gameHeight = 600;
    sf::Vector2f paddleSize(25, 100);
    float ballRadius = 10.f;

    // Create the window of the application
    window.create(sf::VideoMode(gameWidth, gameHeight, 32), "Coroutine Pirates", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Create the left paddle
    sf::RectangleShape leftPaddle;
    leftPaddle.setSize(paddleSize - sf::Vector2f(3, 3));
    leftPaddle.setOutlineThickness(3);
    leftPaddle.setOutlineColor(sf::Color::Black);
    leftPaddle.setFillColor(sf::Color(100, 100, 200));
    leftPaddle.setOrigin(paddleSize / 2.f);

    loadGuy();
    loadForest();

    // Load the text font
    sf::Font font;
    if (!font.loadFromFile("resources/sansation.ttf"))
        return EXIT_FAILURE;

    // Initialize the pause message
    message.setFont(font);
    message.setCharacterSize(25);
    message.setStyle(sf::Text::Bold);
    message.setFillColor(sf::Color::White);

    // Load the sounds used in the game
    sf::SoundBuffer ballSoundBuffer;
    if (!ballSoundBuffer.loadFromFile("resources/ball.wav"))
        return EXIT_FAILURE;
    ballSound.setBuffer(ballSoundBuffer);

    for (auto& guy : guys)
    {
        animate(guy);
    }

    for (auto& guy : guys)
    {
        ai(guy);
    }

    while (window.isOpen())
    {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Window closed or escape key pressed: exit
            if ((event.type == sf::Event::Closed) ||
                ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window.close();
                break;
            }
        }

        // Clear the window
        //window.clear(sf::Color::Black);
        window.draw(fsprite);

        //ballSound.play();
        //window.draw(leftPaddle);
        //window.draw(message);

        for (const auto& guy : guys)
        {
            draw(guy);
        }

        // Display things on screen
        window.display();

        executeOneFrame();
        std::this_thread::sleep_for(16ms);
    }
}
