#include "shape.h"

void Shape_Buffer(const FShape Shape, U32* IndexBuffer, U32* VertexBuffer, U32* ColorBuffer, U32* TexCoordBuffer, U32* NormalBuffer, const U32 ShaderProgram) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, FVector_GetSize(Shape.Indices) * SIZE_INDEX, FVector_Begin(Shape.Indices), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, *VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, FVector_GetSize(Shape.Vertices) * SIZE_VERTEX, FVector_Begin(Shape.Vertices), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, SIZE_VERTEX * 3, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, *ColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, SIZE_COLOR * FVector_GetSize(Shape.Colors), FVector_Begin(Shape.Colors), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, *TexCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, SIZE_TEXCOORD * FVector_GetSize(Shape.TexCoords), FVector_Begin(Shape.TexCoords), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, *NormalBuffer);
    glBufferData(GL_ARRAY_BUFFER, SIZE_NORMAL * FVector_GetSize(Shape.Normals), FVector_Begin(Shape.Normals), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(3);

    // Todo: Update material.
    glUniform1f(glGetUniformLocation(ShaderProgram, "mat.specularStrength"), 0.5f);
    glUniform1f(glGetUniformLocation(ShaderProgram, "mat.shininess"), 0.5f);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
