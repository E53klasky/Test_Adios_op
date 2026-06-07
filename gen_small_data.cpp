#include <adios2.h>
#include <iostream>
#include <vector>
#include <cmath>

int main()
{
    try
    {
        adios2::ADIOS adios;
        adios2::IO io = adios.DeclareIO("SmallIO");
        io.SetEngine("BP5");

        const int t = 2;
        const int ny = 10;
        const int nx = 10;

        auto var = io.DefineVariable<float>(
            "wave_3d",
            { (size_t)t, (size_t)ny, (size_t)nx },
            { 0, 0, 0 },
            { (size_t)t, (size_t)ny, (size_t)nx });

        adios2::Engine writer = io.Open("test_small_data.bp", adios2::Mode::Write);

        for (int step = 0; step < 2; ++step)
        {
            std::vector<float> data(t * ny * nx);
            for (size_t i = 0; i < data.size(); ++i)
                data[i] = std::sin(i + step * 0.1f);

            writer.BeginStep();
            writer.Put(var, data.data());
            writer.EndStep();
            std::cout << "Written step " << step << " size=" << data.size() * sizeof(float) << " bytes" << std::endl;
        }

        writer.Close();
        std::cout << "Done. File: test_small_data.bp" << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
