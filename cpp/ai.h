#ifndef AI_H
#define AI_H

// torch
#undef slots
#include <torch/script.h>
#include <ATen/ATen.h>
#define slots Q_SLOTS
#include <iostream>
#include <memory>
#include <vector>

class AI
{
public:
    AI(std::string modelPath);
    torch::jit::script::Module module;
    std::vector<float> predict(std::vector<float>);

private:
    int inputSize = 5;
    int outputSize = 5;
};

#endif // AI_H
