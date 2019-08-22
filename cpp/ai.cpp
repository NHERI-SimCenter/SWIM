#include "ai.h"

AI::AI(std::string modelPath)
{

    bool modelLoaded = false;

    try
    {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        module = torch::jit::load(modelPath.c_str());
        modelLoaded = true;
        std::cout << modelPath.c_str() << std::endl;
    }
    catch (const c10::Error &e)
    {
        std::cerr << "error loading the AI model\n";
        std::cout << modelPath.c_str() << std::endl;
    }

    if (modelLoaded)
    {
        // Create a vector of inputs.
        std::vector<torch::jit::IValue> inputs;

        at::Tensor inputTensor = torch::ones({1, inputSize});
        inputs.push_back(inputTensor);
        std::cout << inputTensor.slice(1, 0, 6) << '\n';

        // Execute the model and turn its output into a tensor.
        at::Tensor output = module.forward(inputs).toTensor();
        std::cout << output.slice(1, 0, 6) << '\n';

        std::cout << "ok\n";

        std::vector<float> inputsVec {0.9321, 0.1068, 0.2950, 1.0220, 1.0184};//(5, 1.);
        std::vector<float> predValues = predict(inputsVec);
    }

}

std::vector<float> AI::predict(std::vector<float> inputsVec)
{
    int inputSize = inputsVec.size();
    std::vector<float> newCopiedVec(inputsVec);
    float* inputArray = &newCopiedVec[0];

    // Create a vector of inputs.
    std::vector<torch::jit::IValue> inputs;
    at::Tensor inputTensor = torch::from_blob(inputArray,{1, inputSize});
    //std::cout << inputTensor.slice(1, 0, 6) << '\n';
    inputs.push_back(inputTensor);

    // Execute the model and turn its output into a tensor.
    at::Tensor output = module.forward(inputs).toTensor();
    //std::cout << output.slice(1, 0, 6) << '\n';

    std::vector<float> out;
    auto p = static_cast<float*>(output.storage().data());
    for(int i=0; i<output.numel(); i++)
    {
        out.push_back(p[i]);
    }

    return out;

}
