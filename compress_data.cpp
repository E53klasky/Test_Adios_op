#include <iostream>
#include <adios2.h>
#include <vector>

int main() {
    try {
        std::cout << "Compressing data using ADIOS2 with CAESAR" << std::endl;

        adios2::ADIOS adios;

        std::cout << "Opening input file: test_caesar_data.bp" << std::endl;
        adios2::IO readIO = adios.DeclareIO("ReadIO");
        adios2::Engine reader = readIO.Open("test_caesar_data.bp" , adios2::Mode::Read);

        std::cout << "Setting up compressed output file: compressed_output.bp" << std::endl;
        adios2::IO writeIO = adios.DeclareIO("WriteIO");
        writeIO.SetEngine("BP4");
        auto op = adios.DefineOperator("SharedCompressor" , "CAESAR" , { {"error_bound","0.01"},{"mode","CAESAR_V"}, {"batch_size","32"} });
        adios2::Engine writer = writeIO.Open("compressed_output.bp" , adios2::Mode::Write);

        std::cout << "\n=== Processing wave_3d variable ===" << std::endl;

        int step = 0;
        while (reader.BeginStep() == adios2::StepStatus::OK &&  step != 20 ) {

            adios2::Variable<float> varWaveRead = readIO.InquireVariable<float>("wave_3d");

            if (!varWaveRead) {
                std::cerr << "ERROR: Could not find wave_3d variable" << std::endl;
                reader.EndStep();
                continue;
            }

            auto shape = varWaveRead.Shape();
            std::cout << "Step " << step << ": Reading wave_3d with shape [";
            for (size_t i = 0; i < shape.size(); i++) {
                std::cout << shape[i];
                if (i < shape.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;


            // if (shape.size() != 3) {
            //     std::cerr << "ERROR: Expected 3D data, got " << shape.size() << "D" << std::endl;
            //     reader.EndStep();
            //     step++;
            //     continue;
            // }

            // if (shape[0] < 8) {
            //     std::cerr << "ERROR: First dimension must be >= 8 for CAESAR, got " << shape[0] << std::endl;
            //     reader.EndStep();
            //     step++;
            //     continue;
            // }

            std::vector<float> waveData;
            reader.Get(varWaveRead , waveData);
            reader.EndStep();

            if (step == 0) {
                auto varWaveWrite = writeIO.DefineVariable<float>(
                    "wave_3d" ,
                    { shape[0], shape[1], shape[2] } ,
                    { 0, 0, 0 } ,
                    { shape[0], shape[1], shape[2] }
                );

                varWaveWrite.AddOperation(op);
                std::cout << "   Added CAESAR compression to wave_3d" << std::endl;
            }

            auto varWaveWrite = writeIO.InquireVariable<float>("wave_3d");
            writer.BeginStep();
            writer.Put(varWaveWrite , waveData.data());
            writer.EndStep();

            std::cout << "  Compressed and wrote step " << step << std::endl;
            step++;
        }

        std::cout << "\n Successfully compressed " << step << " timesteps" << std::endl;

        reader.Close();
        writer.Close();

        std::cout << "\n========================================" << std::endl;
        std::cout << "Compression complete!" << std::endl;
        std::cout << "Output: compressed_output.bp" << std::endl;
        std::cout << "========================================" << std::endl;

    }
    catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
