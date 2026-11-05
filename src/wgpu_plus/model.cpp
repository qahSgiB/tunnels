#include "wgpu_plus/model.hpp"


#include "glm/mat3x3.hpp"



Model::Model() {}
Model::Model(glm::mat4 model_) : model(model_)
{
    update_model_it();
}

void Model::update_model_it()
{
    model_it = glm::mat3x4(glm::inverse(glm::mat3(model)));
}