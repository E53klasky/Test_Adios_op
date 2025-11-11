#include <iostream>
#include <vector>
#include <limits>
#include <numeric>
#include <adios2.h>

#if ADIOS2_USE_MPI
#include <mpi.h>
#endif

int main(int argc , char** argv)
{
#if ADIOS2_USE_MPI
    MPI_Init(&argc , &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
#else
    MPI_Comm comm = MPI_COMM_NULL;
#endif

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <inputFile> <variableName>" << std::endl;
#if ADIOS2_USE_MPI
        MPI_Finalize();
#endif
        return -1;
    }

    std::string inputFile = argv[1];
    std::string varName = argv[2];

#if ADIOS2_USE_MPI
    adios2::ADIOS adios("" , comm);
#else
    adios2::ADIOS adios("");
#endif

    auto io = adios.DeclareIO("Reader");
    auto reader = io.Open(inputFile , adios2::Mode::Read);

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

        reader.Get(var , data.data() , adios2::Mode::Sync);

        float minVal = std::numeric_limits<float>::max();
        float maxVal = std::numeric_limits<float>::lowest();
        for (float v : data)
        {
            if (v < minVal)
                minVal = v;
            if (v > maxVal)
                maxVal = v;
        }

        std::cout << "Step " << reader.CurrentStep()
            << " -> min = " << minVal
            << ", max = " << maxVal << std::endl;

        reader.EndStep();
    }

    reader.Close();
#if ADIOS2_USE_MPI
    MPI_Finalize();
#endif

    return 0;
}
