#ifndef DIGITIZATION_MANAGER_H
#define DIGITIZATION_MANAGER_H

#include "ScintillationDigitizer.h"
#include "SiPMDigitizer.h"
#include "ADCDigitizer.h"
#include "TotalDigitizer.h"
#include <memory>
#include <string>

class DigitizationManager {
public:
    // 构造函数
    DigitizationManager();
    ~DigitizationManager() = default;
    
    // 设置随机数种子
    void setRandomSeed(unsigned int seed);
    
    // 设置事件数
    void setNumberOfEvents(int events) { nEvents = events; }
    
    // 加载参数文件
    bool loadParameters(const std::string& filename);
    
    // 保存参数到文件
    bool saveParameters(const std::string& filename);
    
    // 运行单个数字化器
    void runSingleDigitizer(const std::string& digitizerType, 
                           const std::string& outputPrefix = "");
    
    // 运行所有数字化器
    void runAllDigitizers(const std::string& outputPrefix = "");
    
    // 扫描参数
    void scanParameter(const std::string& paramName, 
                      const std::vector<double>& values,
                      const std::string& outputDir = "param_scan");
                      
    // 获取参数值
    double getParameter(const std::string& name);
    
    // 设置参数值
    void setParameter(const std::string& name, double value);
    
    // 打印参数
    void printParameters();
    
    // 能量点配置方法
    void setEnergyPoints(const std::vector<double>& energies);
    bool loadEnergyPointsFromFile(const std::string& filename);
    
    // 配置文件加载方法
    bool loadFromConfigFile(const std::string& filename);
    
    // 添加均匀抽样相关方法
    void enableUniformSampling(bool enable = true);
    void setSamplingRange(double minEnergy, double maxEnergy);
    
    // 更新所有数字化器的参数
    void updateDigitizersParameters();
    
private:
    // 数字化器实例
    std::unique_ptr<ScintillationDigitizer> scinDigitizer;
    std::unique_ptr<SiPMDigitizer> sipmDigitizer;
    std::unique_ptr<ADCDigitizer> adcDigitizer;
    std::unique_ptr<TotalDigitizer> totalDigitizer;
    
    // 事件数
    int nEvents;
};

#endif // DIGITIZATION_MANAGER_H 