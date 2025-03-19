#include "DigitizationManager.h"
#include <iostream>
#include <string>
#include <vector>

void printUsage() {
    std::cout << "Usage: digitize [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                     显示帮助信息" << std::endl;
    std::cout << "  -n, --events <number>          设置模拟事件数 (默认: 100000)" << std::endl;
    std::cout << "  -s, --seed <number>            设置随机数种子 (默认: 0)" << std::endl;
    std::cout << "  -d, --digitizer <type>         运行指定的数字化器 (Scintillation/SiPM/ADC/Total)" << std::endl;
    std::cout << "  -a, --all                      运行所有数字化器" << std::endl;
    std::cout << "  -o, --output <prefix>          设置输出文件前缀" << std::endl;
    std::cout << "  -p, --param <name> <value>     设置参数值" << std::endl;
    std::cout << "  -l, --load <file>              从文件加载参数" << std::endl;
    std::cout << "  -w, --save <file>              保存参数到文件" << std::endl;
    std::cout << "  --scan <name> <v1> <v2> ...    扫描参数值" << std::endl;
    std::cout << "  --print-params                 打印所有参数" << std::endl;
    std::cout << "  -e, --energy-points <file>     从文件加载能量点" << std::endl;
    std::cout << "  -c, --config <file>            从配置文件加载所有参数" << std::endl;
    std::cout << "  --energy <e1> <e2> ...         设置模拟能量点 (MeV)" << std::endl;
    std::cout << "  --uniform-sampling             启用均匀能量抽样" << std::endl;
    std::cout << "  --sampling-range <min> <max>   设置均匀抽样的能量范围 (MeV)" << std::endl;
}

int main(int argc, char* argv[]) {
    DigitizationManager manager;
    
    std::string digitizerType;
    std::string outputPrefix = "digi_out";
    bool runAll = false;
    
    bool uniformSampling = false;
    double samplingMinEnergy = 0.0;
    double samplingMaxEnergy = 0.0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        }
        else if (arg == "-n" || arg == "--events") {
            if (i + 1 < argc) {
                manager.setNumberOfEvents(std::stoi(argv[++i]));
            }
        }
        else if (arg == "-s" || arg == "--seed") {
            if (i + 1 < argc) {
                manager.setRandomSeed(std::stoi(argv[++i]));
            }
        }
        else if (arg == "-d" || arg == "--digitizer") {
            if (i + 1 < argc) {
                digitizerType = argv[++i];
            }
        }
        else if (arg == "-a" || arg == "--all") {
            runAll = true;
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputPrefix = argv[++i];
            }
        }
        else if (arg == "-p" || arg == "--param") {
            if (i + 2 < argc) {
                std::string name = argv[++i];
                double value = std::stod(argv[++i]);
                manager.setParameter(name, value);
            }
        }
        else if (arg == "-l" || arg == "--load") {
            if (i + 1 < argc) {
                manager.loadParameters(argv[++i]);
            }
        }
        else if (arg == "-w" || arg == "--save") {
            if (i + 1 < argc) {
                manager.saveParameters(argv[++i]);
            }
        }
        else if (arg == "--scan") {
            if (i + 2 < argc) {
                std::string paramName = argv[++i];
                std::vector<double> values;
                
                while (i + 1 < argc && argv[i+1][0] != '-') {
                    values.push_back(std::stod(argv[++i]));
                }
                
                if (!values.empty()) {
                    manager.scanParameter(paramName, values);
                }
            }
        }
        else if (arg == "--print-params") {
            manager.printParameters();
        }
        else if (arg == "-e" || arg == "--energy-points") {
            if (i + 1 < argc) {
                manager.loadEnergyPointsFromFile(argv[++i]);
            }
        }
        else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                std::string configFile = argv[++i];
                std::cout << "加载配置文件: " << configFile << std::endl;
                if (!manager.loadFromConfigFile(configFile)) {
                    std::cerr << "警告: 无法加载配置文件: " << configFile << std::endl;
                }
            }
        }
        else if (arg == "--energy") {
            std::vector<double> energies;
            while (i + 1 < argc && argv[i+1][0] != '-') {
                energies.push_back(std::stod(argv[++i]));
            }
            
            if (!energies.empty()) {
                manager.setEnergyPoints(energies);
            }
        }
        else if (arg == "--uniform-sampling") {
            uniformSampling = true;
        }
        else if (arg == "--sampling-range") {
            if (i + 2 < argc) {
                samplingMinEnergy = std::stod(argv[++i]);
                samplingMaxEnergy = std::stod(argv[++i]);
            }
        }
    }
    
    // 设置均匀抽样选项
    if (uniformSampling) {
        manager.enableUniformSampling();
        if (samplingMinEnergy > 0 && samplingMaxEnergy > 0) {
            manager.setSamplingRange(samplingMinEnergy, samplingMaxEnergy);
        }
    }
    
    // 执行指定操作
    if (runAll) {
        manager.runAllDigitizers(outputPrefix);
    }
    else if (!digitizerType.empty()) {
        manager.runSingleDigitizer(digitizerType, outputPrefix);
    }
    else if (argc == 1) {
        // 如果没有参数，显示帮助
        printUsage();
    }
    
    return 0;
} 