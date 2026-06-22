#include <iostream>
#include <vector>
#include <limits>
#include <numeric>
#include <algorithm>
#include <adios2.h>
#include <cmath>

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <inputFile_compressed> <original_input> <variableName>" << std::endl;
        return -1;
    }

    std::string inputFile = argv[1];    // compressed file with reconstructed data
    std::string originalFile = argv[2]; // original file with uncompressed data for comparison
    std::string varName = argv[3];

    adios2::ADIOS adios;

    auto io = adios.DeclareIO("Reader");
    auto reader = io.Open(inputFile, adios2::Mode::Read);
    auto originalIO = adios.DeclareIO("OriginalReader");
    auto originalReader = originalIO.Open(originalFile, adios2::Mode::Read);

    while (true)
    {
        auto status = reader.BeginStep();
        auto originalStatus = originalReader.BeginStep();

        if (originalStatus != adios2::StepStatus::OK)
        {
            std::cerr << "No more steps in original file!" << std::endl;
            break;
        }
        if (status != adios2::StepStatus::OK)
            break;

        auto var = io.InquireVariable<float>(varName);
        auto originalVar = originalIO.InquireVariable<float>(varName);

        if (!var)
        {
            std::cerr << "Variable " << varName << " not found in compressed file!" << std::endl;
            break;
        }
        if (!originalVar)
        {
            std::cerr << "Variable " << varName << " not found in original file!" << std::endl;
            break;
        }

        // --- Read reconstructed (compressed) data ---
        std::size_t totalSize = 1;
        for (auto s : var.Shape())
            totalSize *= s;
        std::vector<float> reconstructed(totalSize);
        reader.Get(var, reconstructed.data(), adios2::Mode::Sync);

        // --- Read original data ---
        std::size_t origSize = 1;
        for (auto s : originalVar.Shape())
            origSize *= s;
        std::vector<float> original(origSize);
        originalReader.Get(originalVar, original.data(), adios2::Mode::Sync);

        if (original.size() != reconstructed.size())
        {
            std::cerr << "Step compressed: " << reader.CurrentStep()
                       << ": size mismatch! original=" << original.size()
                       << ", reconstructed=" << reconstructed.size() << std::endl;
            reader.EndStep();
            originalReader.EndStep();
            continue;
        }

        // --- Stats on reconstructed data ---
        float minVal = std::numeric_limits<float>::max();
        float maxVal = std::numeric_limits<float>::lowest();
        for (float v : reconstructed)
        {
            if (v < minVal) minVal = v;
            if (v > maxVal) maxVal = v;
        }

        double sum = std::accumulate(reconstructed.begin(), reconstructed.end(), 0.0);
        double avg = sum / reconstructed.size();

        // Use a sorted copy for median so 'reconstructed' order is preserved for MSE
        std::vector<float> sortedData(reconstructed);
        std::sort(sortedData.begin(), sortedData.end());
        double median;
        if (sortedData.size() % 2 == 0)
            median = 0.5 * (sortedData[sortedData.size() / 2 - 1] + sortedData[sortedData.size() / 2]);
        else
            median = sortedData[sortedData.size() / 2];

        std::cout << "Step compressed: " << reader.CurrentStep()
                   << " -> min = " << minVal
                   << ", max = " << maxVal
                   << ", avg = " << avg
                   << ", median = " << median
                   << std::endl;

        // --- NRMSE between original and reconstructed ---
        double mse = 0.0;
        for (size_t i = 0; i < original.size(); ++i)
        {
            double diff = static_cast<double>(original[i]) -
                          static_cast<double>(reconstructed[i]);
            mse += diff * diff;
        }
        mse /= original.size();

        double rmse = std::sqrt(mse);

        auto [minIt, maxIt] = std::minmax_element(original.begin(), original.end());
        double range = static_cast<double>(*maxIt) - static_cast<double>(*minIt);

        double nrmse = (range != 0.0) ? (rmse / range) : 0.0;

        std::cout << "Step compressed: " << reader.CurrentStep()
                   << ", NRMSE = " << nrmse
                   << std::endl;

        reader.EndStep();
        originalReader.EndStep();
    }

    reader.Close();
    originalReader.Close();

    return 0;
}
