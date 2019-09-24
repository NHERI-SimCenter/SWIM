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

        // ['height', 'length', 'thickness', 'E', 'fc']
        std::vector<float> inputsVec {0.9321, 0.1068, 0.2950, 1.0220, 1.0184};//(5, 1.);
        std::vector<float> predValues = predict(inputsVec);
        // ['Ap','An', 'Bn', 'beta','N']
    }

}

std::vector<float> AI::predict(std::vector<float> inputsVec)
{

    // ['height', 'length', 'thickness', 'E', 'fc'] These values are form the training data
    std::vector<float> meanVec {105.488780, 75.411463, 5.349366, 4149.122637, 5.366606};
    std::vector<float> stdVec {57.042406, 31.171167, 1.883212, 475.822644, 1.224391};

    // normalize input
    int inputSize = inputsVec.size();
    for (int i=0; i<inputSize; i++)
    {
        inputsVec[i] = (inputsVec[i] - meanVec[i])/stdVec[i];
    }

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
