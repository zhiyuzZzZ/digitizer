#ifndef SIPM_DIGITIZER_H
#define SIPM_DIGITIZER_H

#include "DigitizationBase.h"

class SiPMDigitizer : public DigitizationBase {
public:
    SiPMDigitizer();
    virtual ~SiPMDigitizer() = default;
    
    // 数字化方法
    double digitize(double energy) override;
    
protected:
    // 覆盖树初始化方法
    void initializeTree() override;
    
    // 添加：覆盖均匀抽样树初始化方法
    void initializeSamplingTree() override;
    
private:
    // 用于存储Tree数据的变量
    double inputEnergy;
    double peSiPM;
    double peSiPMSat;
    double dc;
    double dcCT;
    double peTotal;
    double peTotalGainFluc;
    double peTotalGainFlucPedSub;
    double peTotalGainFlucPedSub_corr;
    double outputEnergy;
};

#endif // SIPM_DIGITIZER_H 