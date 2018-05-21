#pragma once
#include <Box2D\Box2D.h>
#include <glm\glm.hpp>
#include <Gutengine\Vertex.h>
#include <Gutengine\SpriteBatch.h>
#include <Gutengine\GLTexture.h>
#include <vector>

#include "Object.h"
#include "Grid.h"

enum class Team
{
    BLUE,
    RED
};

class Circle : public Object
{
public:
    Circle();
    ~Circle();

    void init(b2World* world,
        const glm::vec2 position,
        const float width,
        const float height,
        Gutengine::GLTexture texture,
        Gutengine::ColorRGBA8 color,
        bool fixedRotation,
        b2BodyType bodyType,
        float density = 1.0f,
        float friction = 0.3f,
        glm::vec4 uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)) override;

    void draw(Gutengine::SpriteBatch& spriteBatch) override;
    void update(Grid &grid) override;
    
    // getter methods
    b2Body* getBody() const { return m_body; };
    b2Fixture* getFixture() const { return m_fixture; };
    const glm::vec2& getDimensions() const { return m_dimensions; };
    const Gutengine::ColorRGBA8& getColor() const { return m_color; };

    // setter methods
    void setColor(const Gutengine::ColorRGBA8 newColor) { m_color = newColor; };
    

    Team getTeam() { return m_team; };
    void setTeam(Team t) { m_team = t; }

private:
    glm::vec4 m_uvRect;
    b2Body* m_body = nullptr;
    b2Fixture* m_fixture = nullptr;
    glm::vec2 m_dimensions;
    float m_speed;
    Team m_team;

    Gutengine::ColorRGBA8 m_color;
    Gutengine::GLTexture m_texture;
};