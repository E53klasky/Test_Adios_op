#include <iostream>
#include <vector>
#include <fstream>
#include <adios2.h>

#if ADIOS2_USE_MPI
#include <mpi.h>
#endif

int main(int argc, char** argv)
{
#if ADIOS2_USE_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
#else
    MPI_Comm comm = MPI_COMM_NULL;
#endif

    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " <inputFile> <variableName> <outputBinFile>" << std::endl;
#if ADIOS2_USE_MPI
        MPI_Finalize();
#endif
        return -1;
    }

    std::string inputFile = argv[1];
    std::string varName = argv[2];
    std::string outputFile = argv[3];

#if ADIOS2_USE_MPI
    adios2::ADIOS adios("", comm);
#else
    adios2::ADIOS adios("");
#endif

    auto io = adios.DeclareIO("Reader");
    auto reader = io.Open(inputFile, adios2::Mode::Read);

    // Open binary output file
    std::ofstream binFile(outputFile, std::ios::binary);
    if (!binFile)
    {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
#if ADIOS2_USE_MPI
        MPI_Finalize();
#endif
        return -1;
    }

    std::cout << "Reading from: " << inputFile << std::endl;
    std::cout << "Variable: " << varName << std::endl;
    std::cout << "Writing to: " << outputFile << std::endl;

    // Read all steps and write sequentially to binary file
    int stepCount = 0;
    while (true)
    {
        auto status = reader.BeginStep();
        if (status != adios2::StepStatus::OK)
            break;

        auto var = io.InquireVariable<float>(varName);
        if (!var)
        {
            std::cerr << "Variable " << varName << " not found!" << std::endl;
            break;
        }

        std::size_t totalSize = 1;
        for (auto s : var.Shape())
            totalSize *= s;

        std::vector<float> data(totalSize);
        reader.Get(var, data.data(), adios2::Mode::Sync);

        // Write float32 data directly to binary file
        binFile.write(reinterpret_cast<const char*>(data.data()), 
                      totalSize * sizeof(float));

        std::cout << "Step " << stepCount << ": wrote " << totalSize 
                  << " float values (" << (totalSize * sizeof(float)) 
                  << " bytes)" << std::endl;

        stepCount++;
        reader.EndStep();
    }

    binFile.close();
    reader.Close();

    std::cout << "\nTotal steps written: " << stepCount << std::endl;
    std::cout << "Output shape: {1, 20, 20, 256, 256}" << std::endl;
    std::cout << "Total elements: " << (1 * 20 * 20 * 256 * 256) << std::endl;
    std::cout << "Total size: " << (1 * 20 * 20 * 256 * 256 * sizeof(float)) 
              << " bytes" << std::endl;

#if ADIOS2_USE_MPI
    MPI_Finalize();
#endif

    return 0;
}
