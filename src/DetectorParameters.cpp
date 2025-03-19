#include "DetectorParameters.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

DetectorParameters::DetectorParameters() {
    initializeDefaultParameters();
    initializeDefaultEnergyPoints();
    initializeSiPMFunctions();
}

void DetectorParameters::initializeDefaultParameters() {
    // 晶体参数
    parameters["EcalMIPEnergy"] = 8.9;
    parameters["EcalCryMipLY"] = 100;
    parameters["EcalCryEffLY"] = 150;
    parameters["EcalCryIntLY"] = 30000;
    parameters["EcalCryIntLYFlu"] = 0.1;
    parameters["EcalCryLOFlu"] = 0.0;
    // parameters["EcalCryAtt"] = parameters["EcalCryEffLY"] / 
    //                           (parameters["EcalCryIntLY"] * parameters["EcalSiPMPDE"]);
    parameters["EcalCryLYUn"] = 0.00;
    
    // SiPM参数
    parameters["EcalSiPMDigiVerbose"] = 1;
    parameters["EcalSiPMPDE"] = 0.25;
    parameters["EcalSiPMPDEFlu"] = 0.10;
    parameters["EcalSiPMDCR"] = 2500000; // Hz
    parameters["EcalSiPMCT"] = 0.12;
    parameters["EcalSiPMNeuFluence"] = 1000000;
    parameters["EcalSiPMGainMean"] = 5;
    parameters["EcalSiPMGainSigma"] = 0.08;
    parameters["EcalSiPMGainMeanFlu"] = 0.15;

    // 触发参数
    parameters["EcalTriggerThreshold"] = 0;
    parameters["EcalTimeInterval"] = 0.00000015; // second
    parameters["EcalRatioTimeInterval"] = 1.0;
    
    // 电子学参数
    parameters["EcalFEENoiseSigma"] = 5;
    parameters["EcalASICNoiseSigma"] = 4;
    parameters["EcalADCError"] = 0.0;
    parameters["EcalMIP_Thre"] = 0.0;
    parameters["ADCbit"] = 13;
    parameters["ADCSwitch"] = 8000;
    parameters["NofGain"] = 3;
    parameters["Pedestal"] = 50;
    parameters["SiPMDigiVerbose"] = 1;
    parameters["TotalGain"] = 300.0;
    parameters["GainRatio_12"] = 30.0;
    parameters["GainRatio_23"] = parameters["TotalGain"] / parameters["GainRatio_12"];
    
    // 重新计算依赖参数
    parameters["EcalCryAtt"] = parameters["EcalCryEffLY"] / 
                              (parameters["EcalCryIntLY"] * parameters["EcalSiPMPDE"]);
}

double DetectorParameters::getParameter(const std::string& name) const {
    auto it = parameters.find(name);
    if (it != parameters.end()) {
        return it->second;
    } else {
        std::cerr << "Warning: Parameter '" << name << "' not found. Returning 0." << std::endl;
        return 0.0;
    }
}

void DetectorParameters::setParameter(const std::string& name, double value) {
    parameters[name] = value;
    
    // 如果修改了依赖参数，更新计算值
    if (name == "EcalCryMipLY" || name == "EcalCryIntLY" || 
        name == "EcalSiPMPDE" || name == "EcalMIPEnergy") {
        parameters["EcalCryAtt"] = parameters["EcalCryEffLY"] / 
                              (parameters["EcalCryIntLY"] * parameters["EcalSiPMPDE"]);
    }
    
    if (name == "GainRatio_12") {
        parameters["GainRatio_23"] = parameters["TotalGain"] / value;
    }
    
    // 如果修改了SiPM CT参数，更新响应函数
    if (name == "EcalSiPMCT") {
        updateSiPMResponseParameters();
    }
}

void DetectorParameters::printParameters() const {
    std::cout << "=== Detector Parameters ===" << std::endl;
    for (const auto& param : parameters) {
        std::cout << std::setw(25) << std::left << param.first << " = " << param.second << std::endl;
    }
    std::cout << "===========================" << std::endl;
}

bool DetectorParameters::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open parameter file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // 跳过注释和空行
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string name;
        double value;
        
        if (iss >> name >> value) {
            setParameter(name, value);
        }
    }
    
    file.close();
    return true;
}

bool DetectorParameters::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return false;
    }
    
    file << "# Detector Parameters\n";
    file << "# Format: <parameter_name> <value>\n\n";
    
    for (const auto& param : parameters) {
        file << std::setw(25) << std::left << param.first << " " << param.second << "\n";
    }
    
    file.close();
    return true;
}

void DetectorParameters::initializeDefaultEnergyPoints() {
    m_energyPoints = {10, 20, 50, 100, 200, 500};  // MeV
}

void DetectorParameters::setEnergyPoints(const std::vector<double>& energies) {
    m_energyPoints = energies;
}

const std::vector<double>& DetectorParameters::getEnergyPoints() const {
    return m_energyPoints;
}

bool DetectorParameters::loadEnergyPointsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开能量点文件: " << filename << std::endl;
        return false;
    }
    
    std::vector<double> energies;
    std::string line;
    
    while (std::getline(file, line)) {
        // 跳过空行和注释行
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::istringstream iss(line);
        double energy;
        if (iss >> energy) {
            energies.push_back(energy);
        }
    }
    
    if (energies.empty()) {
        std::cerr << "警告: 文件中没有找到有效的能量点" << std::endl;
        return false;
    }
    
    // 设置能量点
    setEnergyPoints(energies);
    
    std::cout << "从文件 " << filename << " 加载了 " << energies.size() << " 个能量点" << std::endl;
    return true;
}

bool DetectorParameters::loadFromConfigFile(const std::string& filename) {
    std::cout << "正在加载配置文件: " << filename << std::endl;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "错误: 无法打开配置文件: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    int lineNum = 0;
    bool hasEnergyPoints = false;
    std::vector<double> energyPoints;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        // 跳过空行和注释行
        if (line.empty() || line[0] == '#') continue;
        
        // 解析参数
        std::istringstream iss(line);
        std::string name;
        std::string equals;
        
        if (!(iss >> name >> equals)) {
            std::cerr << "警告: 第" << lineNum << "行格式无效: " << line << std::endl;
            continue;
        }
        
        if (equals != "=") {
            std::cerr << "警告: 第" << lineNum << "行缺少等号: " << line << std::endl;
            continue;
        }
        
        // 特殊处理能量点
        if (name == "EnergyPoints") {
            hasEnergyPoints = true;
            
            // 读取剩余行作为能量点列表
            std::string valueStr;
            std::getline(iss, valueStr);  // 读取剩余行
            
            std::istringstream valueStream(valueStr);
            std::string token;
            while (std::getline(valueStream, token, ',')) {
                // 去除前后空格
                token.erase(0, token.find_first_not_of(" \t"));
                token.erase(token.find_last_not_of(" \t") + 1);
                
                if (!token.empty()) {
                    try {
                        double value = std::stod(token);
                        energyPoints.push_back(value);
                        std::cout << "添加能量点: " << value << " MeV" << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "警告: 无法解析能量点值: " << token << std::endl;
                    }
                }
            }
        }
        else {
            // 常规参数
            double value;
            if (iss >> value) {
                setParameter(name, value);
                std::cout << "设置参数: " << name << " = " << value << std::endl;
            } else {
                std::cerr << "警告: 无法解析参数值: " << line << std::endl;
            }
        }
    }
    
    // 如果找到能量点，设置它们
    if (hasEnergyPoints && !energyPoints.empty()) {
        setEnergyPoints(energyPoints);
        std::cout << "设置了 " << energyPoints.size() << " 个能量点" << std::endl;
    }
    
    file.close();
    std::cout << "配置文件加载完成: " << filename << std::endl;
    return true;
}

std::vector<std::string> DetectorParameters::getAllParameterNames() const {
    std::vector<std::string> names;
    names.reserve(parameters.size());
    
    for (const auto& [name, value] : parameters) {
        names.push_back(name);
    }
    
    return names;
}

void DetectorParameters::initializeSiPMFunctions() {
    // 初始化SiPM响应函数
    f_SiPMResponse = std::make_unique<TF1>("f_SiPMResponse", 
        "((1-[1])*[0]*(1-exp(-x/[0]))+[1]*x)*([2]+1)/([2]+x/([0]*(1-exp(-x/[0]))))*(1+[3]*exp(-x/[0]))", 
        0, 1e+9);
    
    // 设置默认参数
    // f_SiPMResponse->SetParameters(6.19783e+06, 5.08847e-01, 1.27705e+01, parameters["EcalSiPMCT"]);
    f_SiPMResponse->SetParameters(1.47821e+05, 2.81116e-01, 1.55157e+01, parameters["EcalSiPMCT"]);

    // 初始化SiPM探测器分辨率函数
    f_SiPMSigmaDet = std::make_unique<TF1>("f_SiPMSigmaDet", "8.90971e-01*sqrt(x+5.47081e-01)", 0, 1e+9);
    
    // 初始化SiPM响应上下限函数
    f_SiPMSigmaRecp = std::make_unique<TF1>("f_SiPMSigmaRecp", "f_SiPMResponse+f_SiPMSigmaDet", 0, 1e+9);
    f_SiPMSigmaRecm = std::make_unique<TF1>("f_SiPMSigmaRecm", "f_SiPMResponse-f_SiPMSigmaDet", 0, 1e+9);
    
    // 初始化非对称高斯函数
    f_AsymGauss = std::make_unique<TF1>("AsymGauss", 
        [](double *x, double *par) -> double {
            double val = x[0];
            double mean = par[0];
            double sigma_left = par[1];
            double sigma_right = par[2];

            if (val < mean) {
                return exp(-0.5 * pow((val - mean) / sigma_left, 2));
            } else {
                return exp(-0.5 * pow((val - mean) / sigma_right, 2));
            }
        }, 
        -10, 10, 3);
}

TF1* DetectorParameters::getSiPMResponseFunction() {
    return f_SiPMResponse.get();
}

TF1* DetectorParameters::getSiPMSigmaDetFunction() {
    return f_SiPMSigmaDet.get();
}

TF1* DetectorParameters::getSiPMSigmaRecpFunction() {
    return f_SiPMSigmaRecp.get();
}

TF1* DetectorParameters::getSiPMSigmaRecmFunction() {
    return f_SiPMSigmaRecm.get();
}

TF1* DetectorParameters::getAsymGaussFunction() {
    return f_AsymGauss.get();
}

void DetectorParameters::updateSiPMResponseParameters() {
    // 更新SiPM响应函数的参数
    if (f_SiPMResponse) {
        f_SiPMResponse->SetParameter(3, parameters["EcalSiPMCT"]);
    }
} 