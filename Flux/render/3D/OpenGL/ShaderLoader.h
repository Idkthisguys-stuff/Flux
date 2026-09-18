#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader {
    private:
        unsigned int program = 0;
        std::string vertPath, fragPath, geomPath;
        unsigned int compile(unsigned int type, const std::string& src, const std::string& debugName);
        void link(unsigned int vs, unsigned int fs, unsigned int gs = 0);
        int  location(const std::string& name) const;
    public:
        Shader() = default;
        Shader(const std::string& vertPath, const std::string& fragPath);
        Shader(const std::string& vertPath, const std::string& fragPath, const std::string& geomPath);

        void Bind() const;
        void Reload();

        void Set(const std::string& name, int v) const;
        void Set(const std::string& name, float v) const;
        void Set(const std::string& name, glm::vec3 v) const;
        void Set(const std::string& name, glm::mat3 v) const;
        void Set(const std::string& name, glm::mat4 v) const;

        unsigned int ID() const {return program;}
};