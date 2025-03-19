#ifndef DETECTOR_PARAMETERS_H
#define DETECTOR_PARAMETERS_H

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <TF1.h>
#include <memory>

class DetectorParameters {
public:
    // 单例模式获取实例
    static DetectorParameters& getInstance() {
        static DetectorParameters instance;
        return instance;
    }

    // 参数获取方法
    double getParameter(const std::string& name) const;
    
    // 参数设置方法
    void setParameter(const std::string& name, double value);
    
    // 打印所有参数
    void printParameters() const;
    
    // 从文件加载参数
    bool loadFromFile(const std::string& filename);
    
    // 保存参数到文件
    bool saveToFile(const std::string& filename) const;

    // 添加新方法
    void setEnergyPoints(const std::vector<double>& energies);
    const std::vector<double>& getEnergyPoints() const;
    
    // 从文件加载能量点
    bool loadEnergyPointsFromFile(const std::string& filename);
    
    // 添加配置文件加载方法声明
    bool loadFromConfigFile(const std::string& filename);

    // 添加获取所有参数名称的方法
    std::vector<std::string> getAllParameterNames() const;
    
    // ===== 添加SiPM响应函数 =====
    
    // 获取SiPM响应函数
    TF1* getSiPMResponseFunction();
    
    // 获取SiPM探测器分辨率函数
    TF1* getSiPMSigmaDetFunction();
    
    // 获取SiPM响应上限函数
    TF1* getSiPMSigmaRecpFunction();
    
    // 获取SiPM响应下限函数
    TF1* getSiPMSigmaRecmFunction();
    
    // 获取非对称高斯函数
    TF1* getAsymGaussFunction();
    
    // 更新SiPM响应函数参数
    void updateSiPMResponseParameters();

private:
    // 私有构造函数（单例模式）
    DetectorParameters();
    
    // 禁止拷贝和赋值
    DetectorParameters(const DetectorParameters&) = delete;
    DetectorParameters& operator=(const DetectorParameters&) = delete;
    
    // 参数存储
    std::map<std::string, double> parameters;
    
    // 添加能量点存储
    std::vector<double> m_energyPoints;
    
    // 初始化默认参数
    void initializeDefaultParameters();
    
    // 初始化默认能量点
    void initializeDefaultEnergyPoints();
    
    // ===== SiPM响应函数 =====
    std::unique_ptr<TF1> f_SiPMResponse;
    std::unique_ptr<TF1> f_SiPMSigmaDet;
    std::unique_ptr<TF1> f_SiPMSigmaRecp;
    std::unique_ptr<TF1> f_SiPMSigmaRecm;
    std::unique_ptr<TF1> f_AsymGauss;
    
    // 初始化SiPM响应函数
    void initializeSiPMFunctions();
};

#endif // DETECTOR_PARAMETERS_H 