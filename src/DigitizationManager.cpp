#include "DigitizationManager.h"
#include "DetectorParameters.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <TFile.h>
#include <TTree.h>
#include <TROOT.h>
#include <cstring>
#include <chrono>

DigitizationManager::DigitizationManager() : nEvents(100000) {
    // 清除ROOT内部缓存的对象
    gROOT->Reset();
    
    // 初始化数字化器
    scinDigitizer = std::make_unique<ScintillationDigitizer>();
    sipmDigitizer = std::make_unique<SiPMDigitizer>();
    adcDigitizer = std::make_unique<ADCDigitizer>();
    totalDigitizer = std::make_unique<TotalDigitizer>();
    
    // 设置随机数种子
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    setRandomSeed(seed);
}

void DigitizationManager::setRandomSeed(unsigned int seed) {
    scinDigitizer->setRandomSeed(seed);
    sipmDigitizer->setRandomSeed(seed);
    adcDigitizer->setRandomSeed(seed);
    totalDigitizer->setRandomSeed(seed);
}

bool DigitizationManager::loadParameters(const std::string& filename) {
    return DetectorParameters::getInstance().loadFromFile(filename);
}

bool DigitizationManager::saveParameters(const std::string& filename) {
    return DetectorParameters::getInstance().saveToFile(filename);
}

double DigitizationManager::getParameter(const std::string& name) {
    return DetectorParameters::getInstance().getParameter(name);
}

void DigitizationManager::setParameter(const std::string& name, double value) {
    DetectorParameters::getInstance().setParameter(name, value);
}

void DigitizationManager::printParameters() {
    DetectorParameters::getInstance().printParameters();
}

void DigitizationManager::setEnergyPoints(const std::vector<double>& energies) {
    // 获取参数实例
    auto& params = DetectorParameters::getInstance();
    
    // 设置参数
    params.setEnergyPoints(energies);
    
    // 设置到所有数字化器
    if (scinDigitizer) {
        scinDigitizer->clearEnergyPoints();
        for (double e : energies) {
            scinDigitizer->addEnergyPoint(e);
        }
    }
    
    if (sipmDigitizer) {
        sipmDigitizer->clearEnergyPoints();
        for (double e : energies) {
            sipmDigitizer->addEnergyPoint(e);
        }
    }
    
    if (adcDigitizer) {
        adcDigitizer->clearEnergyPoints();
        for (double e : energies) {
            adcDigitizer->addEnergyPoint(e);
        }
    }
    
    if (totalDigitizer) {
        totalDigitizer->clearEnergyPoints();
        for (double e : energies) {
            totalDigitizer->addEnergyPoint(e);
        }
    }
    
    std::cout << "设置了 " << energies.size() << " 个能量点" << std::endl;
}

bool DigitizationManager::loadEnergyPointsFromFile(const std::string& filename) {
    // 获取参数实例
    auto& params = DetectorParameters::getInstance();
    
    // 加载能量点
    bool success = params.loadEnergyPointsFromFile(filename);
    
    if (success) {
        // 获取加载的能量点
        const std::vector<double>& energies = params.getEnergyPoints();
        
        // 设置到所有数字化器
        if (scinDigitizer) {
            scinDigitizer->clearEnergyPoints();
            for (double e : energies) {
                scinDigitizer->addEnergyPoint(e);
            }
        }
        
        if (sipmDigitizer) {
            sipmDigitizer->clearEnergyPoints();
            for (double e : energies) {
                sipmDigitizer->addEnergyPoint(e);
            }
        }
        
        if (adcDigitizer) {
            adcDigitizer->clearEnergyPoints();
            for (double e : energies) {
                adcDigitizer->addEnergyPoint(e);
            }
        }
        
        if (totalDigitizer) {
            totalDigitizer->clearEnergyPoints();
            for (double e : energies) {
                totalDigitizer->addEnergyPoint(e);
            }
        }
        
        std::cout << "成功从文件加载了 " << energies.size() << " 个能量点" << std::endl;
    } else {
        std::cerr << "无法从文件 " << filename << " 加载能量点" << std::endl;
    }
    
    return success;
}

bool DigitizationManager::loadFromConfigFile(const std::string& filename) {
    std::cout << "加载配置文件: " << filename << std::endl;
    
    // 获取参数实例
    auto& params = DetectorParameters::getInstance();
    
    // 加载参数
    bool success = params.loadFromConfigFile(filename);
    
    if (success) {
        // 更新所有数字化器的参数
        updateDigitizersParameters();
        
        // 获取能量点并设置到数字化器
        const std::vector<double>& energies = params.getEnergyPoints();
        if (!energies.empty()) {
            setEnergyPoints(energies);
        }
    }
    
    return success;
}

void DigitizationManager::runSingleDigitizer(const std::string& type, const std::string& outputPrefix) {
    std::string outputFile = "digi_out_" + type + ".root";
    if (!outputPrefix.empty()) {
        outputFile = outputPrefix + "_" + type + ".root";
    }
    
    if (type == "Scintillation") {
        scinDigitizer->run(nEvents);
        scinDigitizer->saveResults(outputFile);
    } else if (type == "SiPM") {
        sipmDigitizer->run(nEvents);
        sipmDigitizer->saveResults(outputFile);
    } else if (type == "ADC") {
        adcDigitizer->run(nEvents);
        adcDigitizer->saveResults(outputFile);
    } else if (type == "Total") {
        totalDigitizer->run(nEvents);
        totalDigitizer->saveResults(outputFile);
    } else {
        std::cerr << "未知的数字化器类型: " << type << std::endl;
    }
}

void DigitizationManager::runAllDigitizers(const std::string& outputPrefix) {
    // 运行所有数字化器
    runSingleDigitizer("Scintillation", outputPrefix);
    runSingleDigitizer("SiPM", outputPrefix);
    runSingleDigitizer("ADC", outputPrefix);
    runSingleDigitizer("Total", outputPrefix);
    
    // 提示用户如何生成比较图
    std::cout << "所有数字化器运行完成。" << std::endl;
    std::cout << "使用以下命令生成比较图:" << std::endl;
    std::cout << "python scripts/compare_digitizers.py ";
    std::cout << outputPrefix << "_Scintillation.root ";
    std::cout << outputPrefix << "_SiPM.root ";
    std::cout << outputPrefix << "_ADC.root ";
    std::cout << outputPrefix << "_Total.root ";
    std::cout << "-o " << outputPrefix << "_comparison" << std::endl;
}

void DigitizationManager::scanParameter(const std::string& paramName, 
                                       const std::vector<double>& values,
                                       const std::string& outputDir) {
    // 创建输出目录
    std::filesystem::create_directories(outputDir);
    
    // 保存原始参数值
    double originalValue = getParameter(paramName);
    
    // 记录扫描参数信息到一个文件
    TFile* scanFile = TFile::Open((outputDir + "/scan_" + paramName + ".root").c_str(), "RECREATE");
    
    // 创建参数扫描信息树
    TTree* paramInfoTree = new TTree("parameterInfo", "Parameter Scan Information");
    double paramValue;
    std::string paramNameStr = paramName;
    
    paramInfoTree->Branch("paramName", &paramNameStr);
    paramInfoTree->Branch("paramValue", &paramValue, "paramValue/D");
    
    // 记录扫描的所有参数值
    for (double value : values) {
        paramValue = value;
        paramInfoTree->Fill();
    }
    
    // 保存参数信息树
    paramInfoTree->Write();
    
    // 记录所有能量点
    auto& energyPoints = DetectorParameters::getInstance().getEnergyPoints();
    TTree* energyTree = new TTree("energyPoints", "Energy Points");
    double energyPoint;
    energyTree->Branch("energy", &energyPoint, "energy/D");
    
    for (double e : energyPoints) {
        energyPoint = e;
        energyTree->Fill();
    }
    
    energyTree->Write();
    
    // 为每个参数值运行所有数字化器
    for (double value : values) {
        std::cout << "扫描 " << paramName << " = " << value << std::endl;
        
        // 设置参数值
        setParameter(paramName, value);
        
        // 创建此参数值的输出目录
        std::string valueDir = outputDir + "/" + paramName + "_" + std::to_string(value);
        std::filesystem::create_directories(valueDir);
        
        // 只运行数字化器，不做任何后处理
        runAllDigitizers(valueDir + "/digi");
    }
    
    // 关闭文件
    scanFile->Close();
    delete scanFile;
    
    // 还原原始参数值
    setParameter(paramName, originalValue);
    
    std::cout << "参数扫描完成。结果保存到 " << outputDir << std::endl;
    std::cout << "使用 scripts/plot_scan_results.C 脚本可以生成参数扫描图和分析结果：" << std::endl;
    std::cout << "root -l 'scripts/plot_scan_results.C(\"" << outputDir << "/scan_" << paramName << ".root\", \"" << outputDir << "\")'" << std::endl;
}

// 添加启用均匀抽样的方法
void DigitizationManager::enableUniformSampling(bool enable) {
    scinDigitizer->setUniformSampling(enable);
    sipmDigitizer->setUniformSampling(enable);
    adcDigitizer->setUniformSampling(enable);
    totalDigitizer->setUniformSampling(enable);
}

// 添加设置抽样范围的方法
void DigitizationManager::setSamplingRange(double minEnergy, double maxEnergy) {
    scinDigitizer->setSamplingRange(minEnergy, maxEnergy);
    sipmDigitizer->setSamplingRange(minEnergy, maxEnergy);
    adcDigitizer->setSamplingRange(minEnergy, maxEnergy);
    totalDigitizer->setSamplingRange(minEnergy, maxEnergy);
}

// 更新所有数字化器的参数
void DigitizationManager::updateDigitizersParameters() {
    // 获取最新的参数
    auto& params = DetectorParameters::getInstance();
    
    // 更新SiPM响应函数参数
    params.updateSiPMResponseParameters();
    
    // 重新初始化数字化器（如果需要）
    if (scinDigitizer) {
        scinDigitizer->updateParameters();
    }
    
    if (sipmDigitizer) {
        sipmDigitizer->updateParameters();
    }
    
    if (adcDigitizer) {
        adcDigitizer->updateParameters();
    }
    
    if (totalDigitizer) {
        totalDigitizer->updateParameters();
    }
    
    std::cout << "已更新所有数字化器的参数" << std::endl;
} 