#include "CVertexBufferObject.h"
#include <iostream>

void CVertexBufferObject::createVBO() {
    std::cerr << "Creating VBO..." << std::endl;
    glGenBuffers(1, &vboID);
    dataSize = 0;
    std::cerr << "VBO created with ID: " << vboID << std::endl;
}

void CVertexBufferObject::bindVBO() {
    std::cerr << "Binding VBO with ID: " << vboID << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    std::cerr << "VBO bound." << std::endl;
}

void CVertexBufferObject::addData(void* data, unsigned int dataSize) {
    std::cerr << "Adding data to VBO. Current data size: " << this->dataSize << ", Adding: " << dataSize << std::endl;
    this->dataSize += dataSize;
    std::cerr << "New data size: " << this->dataSize << std::endl;
}

void CVertexBufferObject::uploadDataToGPU(GLenum usageHint) {
    std::cerr << "Uploading data to GPU. Total data size: " << dataSize << std::endl;
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, usageHint);
    std::cerr << "Data uploaded to GPU." << std::endl;
}

void CVertexBufferObject::releaseVBO() {
    std::cerr << "Releasing VBO with ID: " << vboID << std::endl;
    glDeleteBuffers(1, &vboID);
    std::cerr << "VBO released." << std::endl;
}
