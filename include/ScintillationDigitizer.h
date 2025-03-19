#ifndef SCINTILLATION_DIGITIZER_H
#define SCINTILLATION_DIGITIZER_H

#include "DigitizationBase.h"

class ScintillationDigitizer : public DigitizationBase {
public:
    ScintillationDigitizer();
    virtual ~ScintillationDigitizer() = default;
    
    // 实现数字化方法
    virtual double digitize(double energy) override;
    
protected:
    // 覆盖initializeTree方法，添加特定的分支
    void initializeTree() override;
    
    // 添加：覆盖initializeSamplingTree方法
    void initializeSamplingTree() override;
    
private:
    // 用于存储Tree数据的变量
    double inputEnergy;
    double phScin;
    double phScinAtt;
    double phScinAttLYRand;
    double outputEnergy;
};

#endif // SCINTILLATION_DIGITIZER_H 