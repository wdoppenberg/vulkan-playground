//
// Created by wdoppenberg on 29-11-21.
//

#ifndef VULKAN_TEST_LVEGAMEOBJECT_HPP
#define VULKAN_TEST_LVEGAMEOBJECT_HPP

#include "LveModel.hpp"

#include <memory>

namespace lve {

    struct Transform2dComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2() const {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);

            glm::mat2 rotMatrix{{c,  s},
                                {-s, c}};


            glm::mat2 scaleMat{{scale.x, 0.f},
                               {.0f,     scale.y}};
            return rotMatrix * scaleMat;
        }
    };

    class LveGameObject {
    public:
        using id_t = unsigned int;

        static LveGameObject createGameObject() {
            static id_t currentId = 0;
            return LveGameObject{currentId++};
        }

        LveGameObject(const LveGameObject &) = delete;

        LveGameObject &operator=(const LveGameObject &) = delete;

        LveGameObject(LveGameObject &&) = default;

        LveGameObject &operator=(LveGameObject &&) = default;

        id_t getId() const { return id; }

        std::shared_ptr<LveModel> model{};
        glm::vec3 color{};
        Transform2dComponent transform2d{};

    private:
        LveGameObject(id_t objId) : id{objId} {};

        id_t id;
    };
}

#endif //VULKAN_TEST_LVEGAMEOBJECT_HPP
