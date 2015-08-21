#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <QDebug>
#include <QString>
#include <QList>

#include <Thor/Animations.hpp>
#include <STP/TMXLoader.hpp>

#include <GL/gl.h>

const int gravity = 350;
bool onGround = false;
float inAir;
float maxInAir = 0.3f;


void addFrames(thor::FrameAnimation& animation, int y, int xFirst, int xLast, float duration = 1.f)
{
    const int step = (xFirst < xLast) ? +1 : -1;
    xLast += step; // so yLast is excluded in the range

    for (int x = xFirst; x != xLast; x += step)
        animation.addFrame(duration, sf::IntRect(32*x, 32*y, 32, 32));
}

int main()
{
    tmx::TileMap map("/home/oawad/Downloads/sfml/ThorTest/Media/sup2.tmx");
    tmx::Layer &collisionLayer = map.GetLayer("Collision");
    tmx::Layer &backLayer = map.GetLayer("back");
    tmx::Layer &foodLayer = map.GetLayer("Food");

    sf::Vector2i screenDimensions(800,640);
    sf::RenderWindow window(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Animations!");
    window.setVerticalSyncEnabled(true);

    sf::View view;
    view.reset(sf::FloatRect(0, 0, screenDimensions.x, screenDimensions.y));
    view.setViewport(sf::FloatRect(0, 0, 1.0f, 1.0f));

    sf::Vector2f scrollPosition(screenDimensions.x / 2, screenDimensions.y / 2);

    ///////////////////////////////////////////////////////////////////////////////
    /// \brief playerImage
    /// ///////////////////////////////////////////////////////////////////////////
    // Load image that contains animation steps
    sf::Image image;
    if (!image.loadFromFile("/home/oawad/Downloads/sfml/ThorTest/Media/player.png"))
        return 1;
    image.createMaskFromColor(sf::Color::White);

    // Create texture based on sf::Image
    sf::Texture texture;
    if (!texture.loadFromImage(image))
        return 1;

    // Create sprite which is animated
    sf::Sprite sprite(texture);
    sprite.setPosition(100.f, 100.f);

    // Define walk animation
    thor::FrameAnimation walkLeft;
    addFrames(walkLeft, 1, 0, 2);			// Frames 0..7	Right leg moves forward

    thor::FrameAnimation walkRight;
    addFrames(walkRight, 2, 0, 2);			// Frames 0..7	Right leg moves forward

    thor::FrameAnimation standStill;
    addFrames(standStill, 0, 0, 0);

    // Register animations with their corresponding durations
    thor::Animator<sf::Sprite, std::string> animator;
    animator.addAnimation("walkLeft", walkLeft, sf::seconds(1.f));
    animator.addAnimation("walkRight", walkRight, sf::seconds(1.f));
    animator.addAnimation("standStill", standStill, sf::seconds(1.f));

    // Create a sf::Vector2f for player velocity and add to the y variable value gravity
    sf::Vector2f playerVelocity(0, gravity);


    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief lettersImage
    ///////////////////////////////////////////////////////////////////////////////////////
    sf::Image lettersImage;
    if (!lettersImage.loadFromFile("/home/oawad/Downloads/sfml/ThorTest/Media/welcome.png"))
        return 1;

    sf::Texture lettersTexture;
    if (!lettersTexture.loadFromImage(lettersImage))
        return 1;

    // Create sprite which is animated
    sf::Sprite lettersSprite(lettersTexture);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \brief moneyImage
    //////////////////////////////////////////////////////////////////////////////////////////
    sf::Image moneyImage;
    if (!moneyImage.loadFromFile("/home/oawad/Downloads/sfml/ThorTest/Media/RocksDC.png"))
        return 1;

    sf::Texture moneyTexture;
    if (!moneyTexture.loadFromImage(moneyImage))
        return 1;

    // Create sprite which is animated
    sf::Sprite moneySprite(moneyTexture);

    thor::FrameAnimation moneyAnimation;
    addFrames(moneyAnimation, 1, 9, 13);

    thor::Animator<sf::Sprite, std::string> moneyAnimator;
    moneyAnimator.addAnimation("money", moneyAnimation, sf::seconds(2.f));

    tmx::ObjectGroup &moneyGroup = map.GetObjectGroup("money");
    const std::vector<tmx::ObjectGroup::Object> &objects = moneyGroup.GetObjects();
    QList<sf::Sprite> coins;
    foreach (const tmx::ObjectGroup::Object obj, objects)
    {
        sf::Sprite coin;
        coin.setPosition(sf::Vector2f(obj.GetX(), obj.GetY()));
        coin.setTexture(moneyTexture);

        coins.append(coin);
    }

    std::cout << static_cast<tmx::ObjectGroup::Object>(objects.at(0)).GetX() << std::endl;


    // Create clock to measure frame time
    sf::Clock frameClock;

    sprite.setPosition(backLayer.GetTile(1, 3).GetGlobalBounds().top, backLayer.GetTile(1, 3).GetGlobalBounds().left);
    animator.playAnimation("standStill", false);

    int collectedCount = 0;

    // Main loop
    while (window.isOpen())
    {
        // Get the frame elapsed time and restart the clock
        sf::Time dt = frameClock.restart();

        // Handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        sprite.setPosition(sprite.getPosition().x + playerVelocity.x * dt.asSeconds(),
                           sprite.getPosition().y + playerVelocity.y * dt.asSeconds());

        onGround = false;
        for (std::size_t x = 0;x < collisionLayer.GetWidth();x++)
        {
            for (std::size_t y = 0;y < collisionLayer.GetHeight();y++)
            {
                // Affected area
                sf::FloatRect area;

                if (sprite.getGlobalBounds().intersects(collisionLayer.GetTile(x, y).GetGlobalBounds(), area))
                {
                    // Verifying if we need to apply collision to the vertical axis, else we apply to horizontal axis
                    if (area.width > area.height)
                    {
                        if (area.contains({ area.left, sprite.getPosition().y }))
                        {
                            // Up side crash
                            sprite.setPosition({ sprite.getPosition().x, sprite.getPosition().y + area.height });

                        }
                        else
                        {
                            // Down side crash
                            onGround = true;
                            inAir = 0.f;
                            sprite.setPosition({ sprite.getPosition().x, sprite.getPosition().y - area.height });
                        }
                    }
                    else if (area.width < area.height)
                    {
                        if (area.contains({ sprite.getPosition().x + sprite.getGlobalBounds().width - 1.f, area.top + 1.f }))
                        {
                            //Right side crash
                            sprite.setPosition({ sprite.getPosition().x - area.width, sprite.getPosition().y });
                        }
                        else
                        {
                            //Left side crash
                            sprite.setPosition({ sprite.getPosition().x + area.width, sprite.getPosition().y });
                        }
                    }
                }

                sf::FloatRect foodArea;
                if (sprite.getGlobalBounds().intersects(foodLayer.GetTile(x, y).GetGlobalBounds(), foodArea) && foodLayer.GetTile(x, y).visible)
                {
                    foodLayer.GetTile(x, y).visible = false;
                    collectedCount++;
                }
            }

        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            if (animator.isPlayingAnimation() && animator.getPlayingAnimation() == "walkRight")
            {
                animator.stopAnimation();
            }

            if (!animator.isPlayingAnimation())
            {
                animator.playAnimation("walkLeft", false);
            }
            playerVelocity.x = -gravity;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            if (animator.isPlayingAnimation() && animator.getPlayingAnimation() == "walkLeft")
            {
                animator.stopAnimation();
            }

            if (!animator.isPlayingAnimation())
            {
                animator.playAnimation("walkRight", false);
            }
            playerVelocity.x = gravity;
        }
        else if (playerVelocity.x != 0)
        {
            playerVelocity.x = 0;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && (onGround || inAir < maxInAir))
        {
            playerVelocity.y = -gravity;
            inAir += dt.asSeconds();
        }
        else
        {
            playerVelocity.y = gravity;
            inAir = maxInAir;
        }

        if (sprite.getPosition().x + 10 > screenDimensions.x / 2)
            scrollPosition.x = sprite.getPosition().x + 10;
        else
            scrollPosition.x = screenDimensions.x / 2;

        if (scrollPosition.x + screenDimensions.x / 2 > map.GetWidth() * 32)
        {
            scrollPosition.x = map.GetWidth() * 32 - screenDimensions.x / 2;
        }

        if (!moneyAnimator.isPlayingAnimation())
        {
            moneyAnimator.playAnimation("money", false);
        }

        view.setCenter(scrollPosition);

        window.setView(view);

        // Update animator and apply current animation state to the sprite
        animator.update(dt);
        animator.animate(sprite);

        moneyAnimator.update(dt);
        moneyAnimator.animate(moneySprite);

        // Draw everything
        window.clear();
        window.draw(map);
        window.draw(sprite);
        window.draw(moneySprite);

        glEnable (GL_SCISSOR_TEST);
        glScissor(680, 0, 120,  screenDimensions.y);
        window.clear();

        lettersSprite.setTextureRect(sf::IntRect(0, 0, 112, 32));
        lettersSprite.setPosition(sf::Vector2f(scrollPosition.x + 285, 0));

        window.draw(lettersSprite);
        glDisable (GL_SCISSOR_TEST);

        window.display();
    }
}
