/********************************************
Author #1 : LAU YEE KEEN CALVIN (Programmer)
Author #2 : CHAN JIN XUAN (Graphics Design)
********************************************/
#include <box2d.h>
#include <cstdio>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <vector>
#include <iostream>
#include <random>
#include <cstdint>
#include <string>

namespace converter
{
    //Converter to convert from meter to pixel and pixel to meter value

    const float PPM = 32.f;

    float pixelToMeter(float pixel)
    {
        return pixel / PPM;
        //return pixel;
    }

    float meterToPixel(float meter)
    {
        return meter * PPM;
        //return meter;
    }

    float sfmlToBox2dCoordinateY(float y)
    {
        return pixelToMeter(900) - y;
    }

    float box2dToSfmlCoordinateY(float y)
    {
        return pixelToMeter(900) - y;
    }

    float radToDeg(float rad)
    {
        return (rad) * (180 / 3.142f);
    }

} // namespace converter

namespace gameEng
{

    //this namespace contains all the game object and physics engines

    bool isUp = false;
    b2Body *bodyToBeDestroy = nullptr;
    int currentScore = 0;

    enum entityName
    {
        GROUND,
        CHARACTER,
        STONE_BLOCK,
        COIN
    };

    //This entity class is to hold a game objects data like b2Body and its entity type.
    class Entity
    {

        int entityType;
        b2Body *entityBody = nullptr;
        b2World *world = nullptr;
        float width = 0.0f;
        float height = 0.0f;

    public:
        Entity(int entityType, b2Body *entityBody, float width, float height, b2World *world)
        {
            this->entityType = entityType;
            this->entityBody = entityBody;
            this->width = width;
            this->height = height;
            this->world = world;
        }

        int getEntityType()
        {
            return this->entityType;
        }

        b2Body *getEntityBody()
        {
            return this->entityBody;
        }

        float getWidth()
        {
            return this->width;
        }

        float getHeight()
        {
            return this->height;
        }

        b2World *getWorld()
        {
            return this->world;
        }
    };

    //stores all the created game object to be rendered by SFML
    std::vector<Entity> entityList;

    //ContactListener class overrides all the method of b2ContactListener for collision detection callbacks by Box2D
    class ContactListener : public b2ContactListener
    {
    private:
        virtual void BeginContact(b2Contact *contact) //Callback method when 2 object begin to collide
        {

            b2Body *fixtureA = contact->GetFixtureA()->GetBody();
            b2Body *fixtureB = contact->GetFixtureB()->GetBody();

            //check if the Character object is colliding with coin object, then destroy the coin object from the b2world
            for (int i = 0; i < entityList.size(); i++)
            {
                if ((entityList[i].getEntityBody() == fixtureA || entityList[i].getEntityBody() == fixtureB) && entityList[i].getEntityType() == COIN)
                {
                    bodyToBeDestroy = entityList[i].getEntityBody();
                    currentScore++; //increase the score value by 1 whenever the character collides with a coin.
                }
            }
        }

        virtual void EndContact(b2Contact *contact) {}
        virtual void PreSolve(b2Contact *contact, const b2Manifold *oldManifold) {}
        virtual void PostSolve(b2Contact *contact, const b2ContactImpulse *impulse) {}
    };

    //Game class that responsible to create all the game object and also update the position of each of the game object
    class Game
    {

    private:
        //NOTE: Box2d use metrics system

        //definition of the attributes for all of the game objects.
        const float GRAVITY_Y = -10.0f;
        const float GRAVITY_X = 0.0f;

        const float GROUND_HEIGHT = 10.0f;
        const float GROUND_WIDTH = 20.0f;

        const float GROUND_START_POSITION_X = 0.0f;
        const float GROUND_START_POSITION_Y = 0.0f;

        const float STONE_BLOCK_HEIGHT = 2.0f;
        const float STONE_BLOCK_WIDTH = 4.0f;

        const float CHARACTER_HEIGHT = 4.0f;
        const float CHARACTER_WIDTH = 2.0f;

        const float COIN_HEIGHT = 1.0f;
        const float COIN_WIDTH = 1.0f;

        const float DELTA_TIME = 1.0f / 60.0f;

        b2World *myWorld = nullptr;
        b2Body *pCharacter = nullptr;
        b2Body *pCoin = nullptr;

        sf::Texture groundTexture;
        sf::Texture stoneTexture;
        sf::Texture characterTexture;

        sf::SoundBuffer soundBufferCoin;
        sf::Sound coinSound;

    public:
        Game()
        {
            //load all the texture image from file into the sf::Texture object, so that it can be used later during the rendering
            groundTexture.loadFromFile("Assets/blue_box.png");
            groundTexture.setSmooth(true);
            stoneTexture.loadFromFile("Assets/horizontal_box.png");
            stoneTexture.setSmooth(true);
            characterTexture.loadFromFile("Assets/astronaut.png");

            soundBufferCoin.loadFromFile("Assets/coins.wav");
            coinSound.setBuffer(soundBufferCoin);

            b2Vec2 gravity(GRAVITY_X, GRAVITY_Y);

            myWorld = new b2World(gravity);

            createGround(GROUND_WIDTH, GROUND_HEIGHT, GROUND_START_POSITION_X, GROUND_START_POSITION_Y, true);                                    // top ground
            createGround(GROUND_WIDTH, GROUND_HEIGHT, GROUND_START_POSITION_X, GROUND_START_POSITION_Y + 25.0f + GROUND_START_POSITION_Y, false); //bottom ground
            createCharacter(CHARACTER_WIDTH, CHARACTER_HEIGHT, 0.0f - GROUND_WIDTH / 2.0f + CHARACTER_WIDTH / 2.0f, 10.0f);
            createStoneBlock(30.0f, 2.0f, 25.0f, 25.0f); // stairways

            //Create the blocks of stone object at the begining of the game scene
            for (int i = 0; i < 3; i++)
            {
                if (i != 2)
                {
                    createBlockGroup(10.0f + (10.0f * i), 18.0f - (3.0f * i));
                }
                else if (i == 2)
                {
                    createBlockGroup(10.0f + (10.0f * i), 6.978f + (3.0f * i));
                }

                if (i < 2)
                {
                    createBlockGroup(10.0f + (10.0f * i), 7.0f + (3.0f * i));
                }
            }

            createStoneBlock(30.0f, 2.0f, 25.0f, 0.0f);
        }

        //obstacles for the gameplay
        void createObstacles(int initPosX, int initPosY, bool isTop)
        {
            float obstaclesHeight = 2.0f;
            if (isTop)
            {
                obstaclesHeight = 2.0f;
            }
            else
            {
                obstaclesHeight = -2.0f;
            }

            for (int i = 0; i < 2; i++)
            {
                if (i == 1)
                {
                    createStoneBlock(2.0f, 2.0f, initPosX + 10, initPosY + obstaclesHeight);
                }
                else
                {
                    createStoneBlock(12.0f, 2.0f, initPosX + 5, initPosY);
                }
            }

            for (int i = 0; i < 5; i++)
            {

                if (!isTop)
                {
                    if (initPosY == 23.0f)
                    {
                        createCoin(COIN_WIDTH, COIN_HEIGHT, initPosX + (2.0f * i), initPosY - 2.0f);
                    }
                    else if (initPosY == 1.0f)
                    {
                        createCoin(COIN_WIDTH, COIN_HEIGHT, initPosX + (2.0f * i), initPosY + 2.0f);
                    }
                    else if (initPosY == 14.0f)
                    {
                        createCoin(COIN_WIDTH, COIN_HEIGHT, initPosX + (2.0f * i), initPosY + 2.0f);
                    }
                    else if (initPosY == 11.0f)
                    {
                        createCoin(COIN_WIDTH, COIN_HEIGHT, initPosX + (2.0f * i), initPosY - 2.0f);
                    }
                }
            }
        }

        //create groups of stones blocks to form a large platform
        void createBlockGroup(int initPosX, int initPosY)
        {

            createStoneBlock(10.0f, 2.0f, initPosX + 10.0f, initPosY);
        }

        b2Body *createGround(float width, float height, float positionX, float positionY, bool isTop)
        {

            //create ground body top and bottom
            b2BodyDef groundBodyDef;
            groundBodyDef.position.Set(positionX, positionY);
            //add body (groundBodyBottomDef) to world
            b2Body *groundBody = myWorld->CreateBody(&groundBodyDef);
            //define ground body shape
            b2PolygonShape groundShape;
            groundShape.SetAsBox(width / 2.0f, height / 2.0f);
            //create fixture
            groundBody->CreateFixture(&groundShape, 0.0f);

            float dynamicPositionY = 0.0f;
            float dynamicPosX = 0.0f;

            //to create stairways in between the top and bottom of the ground
            if (!isTop)
            {
                for (int i = 0; i < 5; i++)
                {
                    dynamicPosX = ((STONE_BLOCK_WIDTH / 2.0f) + positionX) - GROUND_WIDTH / 2.0f + (STONE_BLOCK_WIDTH * i);
                    if (i <= 2)
                    {
                        dynamicPositionY = ((GROUND_HEIGHT / 2.0f) + STONE_BLOCK_HEIGHT / 2.0f) + (STONE_BLOCK_WIDTH / 2.0f * i);
                    }
                    else
                    {
                        dynamicPositionY -= (STONE_BLOCK_WIDTH / 2.0f);
                    }

                    createStoneBlock(STONE_BLOCK_WIDTH, STONE_BLOCK_HEIGHT, dynamicPosX, dynamicPositionY);
                }
            }

            Entity groundEntity(GROUND, groundBody, width, height, myWorld);

            //push the created object into the vector
            entityList.push_back(groundEntity);

            return groundBody;
        }

        b2Body *createStoneBlock(float width, float height, float positionX, float positionY)
        {

            //body definition properties
            b2BodyDef stoneBlockBodyDef;
            stoneBlockBodyDef.position.Set(positionX, positionY);

            //puts the created body into myWorld
            b2Body *stoneBlockBody = myWorld->CreateBody(&stoneBlockBodyDef);

            //define the shape
            b2PolygonShape stoneBlockShape;
            stoneBlockShape.SetAsBox(width / 2.0f, height / 2.0f);

            //define fixture
            b2FixtureDef b2FixtureDef;
            b2FixtureDef.friction = 0.0f;
            b2FixtureDef.shape = &stoneBlockShape;

            stoneBlockBody->CreateFixture(&b2FixtureDef);

            Entity stoneBlockEntity(STONE_BLOCK, stoneBlockBody, width, height, myWorld);

            //push created object into vector for rendering later on
            entityList.push_back(stoneBlockEntity);

            return stoneBlockBody;
        }

        b2Body *createCharacter(float width, float height, float positionX, float positionY)
        {

            //body definition
            b2BodyDef characterBodyDef;
            characterBodyDef.type = b2_dynamicBody;
            characterBodyDef.position.Set(positionX, positionY);
            characterBodyDef.fixedRotation = true;

            //put created body into myWorld
            b2Body *characterBody = myWorld->CreateBody(&characterBodyDef);
            pCharacter = characterBody;
            characterBody->SetSleepingAllowed(false);

            //define the shape
            b2PolygonShape characterBodyShape;
            characterBodyShape.SetAsBox(width / 2.0f, height / 2.0f);

            //fixture definition
            b2FixtureDef characterFixtureDef;
            characterFixtureDef.shape = &characterBodyShape;
            characterFixtureDef.density = 3.0f;
            characterFixtureDef.friction = 0.0f;

            //create the fixture by inserting the fixture definition
            characterBody->CreateFixture(&characterFixtureDef);

            Entity characterEntity(CHARACTER, characterBody, width, height, myWorld);

            entityList.push_back(characterEntity);

            return characterBody;
        }

        b2Body *createCoin(float width, float height, float positionX, float positionY)
        {

            //body definition
            b2BodyDef coinBodyDef;
            coinBodyDef.type = b2_kinematicBody;
            coinBodyDef.position.Set(positionX, positionY);
            coinBodyDef.fixedRotation = true;

            //insert the body definition into myWorld
            b2Body *coinBody = myWorld->CreateBody(&coinBodyDef);

            //define the shape
            b2PolygonShape coinBodyShape;
            coinBodyShape.SetAsBox(width / 2.0f, height / 2.0f);

            //define fixture
            b2FixtureDef coinFixtureDef;
            coinFixtureDef.shape = &coinBodyShape;
            coinFixtureDef.density = 3.0f;
            coinFixtureDef.friction = 0.0f;

            //create fixture
            coinBody->CreateFixture(&coinFixtureDef);

            Entity coinEntity(COIN, coinBody, width, height, myWorld);

            entityList.push_back(coinEntity);

            return coinBody;
        }

        void update(sf::RenderWindow *window)
        {
            //time steps for the game
            myWorld->Step(DELTA_TIME, 6, 2);

            //destroy the coin body which have collided with the character
            for (int i = 0; i < entityList.size(); i++)
            {
                if (bodyToBeDestroy)
                {
                    if (entityList[i].getEntityBody() == bodyToBeDestroy)
                    {
                        myWorld->DestroyBody(bodyToBeDestroy);
                        entityList.erase(entityList.begin() + i);
                        bodyToBeDestroy = nullptr;
                        if (coinSound.getStatus() != coinSound.Playing)
                        {
                            coinSound.play();
                        }
                    }
                }
            }

            //Render all the created game entities using SFML
            for (auto &entity : entityList)
            {

                int x = converter::meterToPixel(entity.getEntityBody()->GetPosition().x);
                int y = converter::meterToPixel(converter::box2dToSfmlCoordinateY(entity.getEntityBody()->GetPosition().y));
                int width = converter::meterToPixel(entity.getWidth());
                int height = converter::meterToPixel(entity.getHeight());

                if (entity.getEntityType() == GROUND)
                {
                    sf::RectangleShape ground(sf::Vector2f(width, height));
                    ground.setTexture(&groundTexture);
                    ground.setPosition(x, y);
                    ground.setOrigin(width / 2.0f, height / 2.0f);
                    window->draw(ground);
                }
                else if (entity.getEntityType() == CHARACTER)
                {

                    sf::RectangleShape charact(sf::Vector2f(width, height));
                    charact.setTexture(&characterTexture);
                    charact.setPosition(x, y);
                    charact.setOrigin(width / 2.0f, height / 2.0f);
                    charact.setRotation(converter::radToDeg(-entity.getEntityBody()->GetAngle()));
                    window->draw(charact);
                }
                else if (entity.getEntityType() == STONE_BLOCK)
                {

                    sf::RectangleShape stone(sf::Vector2f(width, height));
                    stone.setTexture(&stoneTexture);
                    stone.setPosition(x, y);
                    stone.setOrigin(width / 2.0f, height / 2.0f);
                    window->draw(stone);
                }
                else if (entity.getEntityType() == COIN)
                {

                    sf::CircleShape coin;
                    coin.setRadius(width / 2);
                    coin.setFillColor(sf::Color::Yellow);
                    coin.setPosition(x, y);
                    coin.setOrigin(width / 2.0f, height / 2.0f);
                    window->draw(coin);
                }
            }
        }

        b2World *getMyWorld()
        {
            return myWorld;
        }

        b2Body *getCharacter()
        {
            return pCharacter;
        }

        b2Body *getCoin()
        {
            return pCoin;
        }

        float getGroundHeight()
        {
            return converter::meterToPixel(GROUND_HEIGHT);
        }

        float getGroundWidth()
        {
            return converter::meterToPixel(GROUND_WIDTH);
        }

        float getStoneBlockHeight()
        {
            return converter::meterToPixel(STONE_BLOCK_HEIGHT);
        }

        float getStoneBlockWidth()
        {
            return converter::meterToPixel(STONE_BLOCK_WIDTH);
        }

        float getCharacterHeight()
        {
            return converter::meterToPixel(CHARACTER_HEIGHT);
        }

        float getCharacterWidth()
        {
            return converter::meterToPixel(CHARACTER_WIDTH);
        }
    };

} // namespace gameEng

int main()
{
    //get the screen width and height
    unsigned int screenWidth = sf::VideoMode::getDesktopMode().width;
    unsigned int screenHeight = sf::VideoMode::getDesktopMode().height;
    gameEng::Game game;

    //an random device generator to help generate the obstacles in the game
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 1);

    //Create a window
    sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(screenWidth, screenHeight), "Adam's Adventure");
    window->setFramerateLimit(60);

    //set the viewing position of the window in SFML to fit all the game entity onto screen
    sf::View view2;
    view2.setSize(sf::Vector2f(screenWidth, screenHeight));
    view2.setCenter(screenWidth / 2, screenHeight / 2);
    view2.move(-450.0f, -(game.getGroundHeight() / 2) + 200.0f);
    window->setView(view2);

    //checks on the gameplay status
    bool moveRight = true;
    bool nearEnding = false;
    bool isReady = false;
    bool isWon = false;
    bool isLost = false;

    //assign a contact listener for collision detection in box2d
    gameEng::ContactListener contactListener;
    game.getMyWorld()->SetContactListener(&contactListener);

    //define the text display object provided by SFML
    sf::Text scoreText;
    sf::Font font;
    sf::Texture bgTexture;
    sf::Sprite bgSprite;

    //load all the textures from file
    font.loadFromFile("Font/Changa-VariableFont_wght.ttf");
    bgTexture.loadFromFile("Assets/bg.png");
    bgSprite.setTexture(bgTexture);
    bgSprite.setScale(2, 2);
    bgSprite.setOrigin(bgSprite.getTexture()->getSize().x / 2.0f, bgSprite.getTexture()->getSize().y / 2.0f);
    scoreText.setFont(font);
    scoreText.setCharacterSize(220);
    scoreText.setOrigin(220 / 2.0f, 220 / 2.0f);
    scoreText.setFillColor(sf::Color::Magenta);

    //play sound effect
    sf::SoundBuffer soundBufferVictory;
    soundBufferVictory.loadFromFile("Assets/victory.ogg");
    sf::Sound victorySound;
    victorySound.setBuffer(soundBufferVictory);

    //Game Loop
    while (window->isOpen())
    {

        sf::Event event;

        while (window->pollEvent(event))
        {

            // check the type of the event...
            switch (event.type)
            {
            // window closed
            case sf::Event::Closed:
                window->close();
                break;

            // key pressed
            case sf::Event::KeyPressed:

                //move the object upwards when the up key is being PRESSED
                if (event.key.code == sf::Keyboard::Up)
                {
                    moveRight = false;
                    isReady = true;
                    game.getCharacter()->SetLinearVelocity(b2Vec2(0.0f, 10.0f));
                    game.getMyWorld()->SetGravity(b2Vec2(0.0f, 200.0f));
                }
                //move the object downwards key is pressed move the object downward
                else if (event.key.code == sf::Keyboard::Down)
                {
                    moveRight = false;
                    game.getCharacter()->SetLinearVelocity(b2Vec2(0.0f, -10.0f));
                    game.getMyWorld()->SetGravity(b2Vec2(0.0f, -200.0f));
                }

                break;

            // we don't process other types of events
            default:
                break;
            }

            //when is key released move the object towards the right
            if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::Up)
                {
                    moveRight = true;
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    moveRight = true;
                }
            }
        }

        //Starts to destroy all the entity game object after it is being left out of players sight (behind the screen)
        for (int i = 0; i < gameEng::entityList.size(); i++)
        {
            if (converter::pixelToMeter(view2.getCenter().x) - gameEng::entityList[i].getEntityBody()->GetPosition().x > 62.5f && gameEng::entityList[i].getEntityType() != gameEng::CHARACTER)
            {
                game.getMyWorld()->DestroyBody(gameEng::entityList[i].getEntityBody());
                gameEng::entityList.erase(gameEng::entityList.begin() + i);
            }
        }

        float largestPosX = 0.0f;
        //get the furtherst rendered game object
        for (auto &entity : gameEng::entityList)
        {
            if (entity.getEntityBody()->GetPosition().x > largestPosX)
            {
                largestPosX = entity.getEntityBody()->GetPosition().x;
            }
        }

        //a loop to render the obstacles in the game scene as the character travel throughout the game scene
        if (converter::meterToPixel(largestPosX) - view2.getCenter().x <= 720 && game.getCharacter()->GetPosition().x < 500.0f)
        {

            for (int i = 0; i < 10; i++)
            {
                if (i == 0 && largestPosX >= 90)
                {
                    largestPosX -= 2.0f;
                }
                else
                {
                    largestPosX += 5.f;
                }

                bool isTop = dist(rd);

                //generate top and bottom obstacles based on the largestPosX point
                game.createObstacles(largestPosX + (12.f * i), 23.f, dist(rd));
                if (i == 9)
                {
                    game.createObstacles(largestPosX + (12.f * i) + 8.0f, 14.f, false);
                    game.createObstacles(largestPosX + (12.f * i) + 8.0f, 11.f, true);
                }
                else
                {
                    game.createObstacles(largestPosX + (12.f * i) + 8.0f, 14.f, isTop);
                    game.createObstacles(largestPosX + (12.f * i) + 8.0f, 11.f, !isTop);
                }
                game.createObstacles(largestPosX + (12.f * i), 1.f, dist(rd));
            }
        }
        else if (game.getCharacter()->GetPosition().x >= 500.0f && !nearEnding) //if the game is near ending, render the ending game scene
        {
            nearEnding = true;
            game.createBlockGroup(largestPosX - 10.0f, 23.f);
            game.createBlockGroup(largestPosX - 10.0f, 1.f);
            game.createGround(20.0f, 10.0f, largestPosX + 20.0f, 0.0f, true);
            game.createGround(20.0f, 10.0f, largestPosX + 20.0f, 0.0f + 25.0f + 0.0f, false);
        }

        //keep moving the character towards the right while the game is not won or not lost yet
        if (moveRight && !isWon && !isLost)
        {
            game.getCharacter()->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
        }
        //if won or lost, stop moving everything
        else if (isWon || isLost)
        {
            game.getCharacter()->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            game.getMyWorld()->SetGravity(b2Vec2(0.0f, -10.0f));
        }

        //check if the game is lost
        if (game.getCharacter()->GetPosition().y >= 30.0f || game.getCharacter()->GetPosition().y <= 0.0f)
        {
            //Lost
            scoreText.setString("YOU LOST!");
            scoreText.setPosition(view2.getCenter().x - 550.0f, 220.0f);
            isLost = true;
        }

        if (converter::pixelToMeter(view2.getCenter().x) - game.getCharacter()->GetPosition().x > converter::pixelToMeter(screenWidth / 2.0f) + 2.0f)
        {
            //Lost
            scoreText.setString("YOU LOST!");
            scoreText.setPosition(view2.getCenter().x - 550.0f, 220.0f);
            isLost = true;
        }

        //check if the player has won the game
        if (game.getCharacter()->GetPosition().x >= 574.0f)
        {
            //won
            scoreText.setString("YOU WON! " + std::to_string(gameEng::currentScore));
            scoreText.setPosition(view2.getCenter().x - 550.0f, 420.0f);
            if (victorySound.getStatus() != victorySound.Playing && !isWon)
            {
                victorySound.play();
            }
            isWon = true;
        }

        //whenever the player is ready, start to move the object towards the right
        if (isReady && !isWon && !isLost)
        {
            view2.move(4.3f, 0.0f);
            window->setView(view2);
        }

        window->clear(sf::Color::White);

        //while the player is playing the game, print out the current score
        if (!isWon && !isLost)
        {
            scoreText.setString(std::to_string(gameEng::currentScore));
            scoreText.setPosition(view2.getCenter().x, 220.0f);
        }

        bgSprite.setPosition(view2.getCenter().x, view2.getCenter().y);
        window->draw(bgSprite);
        window->draw(scoreText);
        game.update(window); //update the game for each loop
        window->display();
    }

    return 0;
}
