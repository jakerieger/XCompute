// Author: Jake Rieger
// Created: 7/29/2025.
//

#pragma once

namespace x {
    inline static auto kVertexShaderSource = R"(#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
    )";

    inline static auto kFragmentShaderSource = R"(#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D uOutputTexture;

void main() {
    FragColor = texture(uOutputTexture, TexCoord);
}
    )";
}  // namespace x
