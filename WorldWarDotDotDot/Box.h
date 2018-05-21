#pragma once

#include <Box2D\Box2D.h>
#include <glm\glm.hpp>
#include <Gutengine\Vertex.h>
#include <Gutengine\SpriteBatch.h>
#include <Gutengine\GLTexture.h>
#include <vector>

#include "Object.h"

class Box : public Object
{
public:
    Box();
    ~Box();

    void init(b2World* world,
        const glm::vec2 position,
        const float width,
        const float height,
        Gutengine::GLTexture texture,
        Gutengine::ColorRGBA8 color,
        bool fixedRotation,
        b2BodyType bodyType,
        float density,
        float friction,
        glm::vec4 uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)) override;

   

    void draw(Gutengine::SpriteBatch& spriteBatch) override;

    void virtual update(Grid &grid) override;

    // getter methods
    b2Body* getBody() const { return m_body; };
    b2Fixture* getFixture() const { return m_fixture; };
    const glm::vec2& getDimensions() const { return m_dimensions; };
    const Gutengine::ColorRGBA8& getColor() const { return m_color; };

    const std::vector<glm::vec2> getCorners() const;

private:
    glm::vec4 m_uvRect;
    b2Body* m_body = nullptr;
    b2Fixture* m_fixture = nullptr;
    glm::vec2 m_dimensions;

    std::vector<glm::vec2> m_corners;
    Gutengine::ColorRGBA8 m_color;
    Gutengine::GLTexture m_texture;

};