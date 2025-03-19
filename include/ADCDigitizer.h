#ifndef ADC_DIGITIZER_H
#define ADC_DIGITIZER_H

#include "DigitizationBase.h"

class ADCDigitizer : public DigitizationBase {
public:
    ADCDigitizer();
    virtual ~ADCDigitizer() = default;
    
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
    double adcIni;
    double adcGainMean;
    double adcGainSigma;
    double adcGain;
    int gainRange;
    double outputEnergy;
};

#endif // ADC_DIGITIZER_H 